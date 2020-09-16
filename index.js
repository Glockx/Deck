const SerialPort = require("serialport");
const Readline = require("@serialport/parser-readline");
const OBSWebSocket = require("obs-websocket-js");
const TwitchClient = require("twitch").default;
const { TaskTimer } = require("tasktimer");
const open = require("open");
const { audio } = require("system-control");

//  ===================================================== Constant =================================================================
// Name of Arudino port on my Mac. please change it if you want to use it.
var arduinoCOMPort = "/dev/cu.usbserial-14610";
const obs = new OBSWebSocket();
const arduinoSerialPort = new SerialPort(arduinoCOMPort, { baudRate: 9600 });

const parser = new Readline({
  delimiter: "\n",
});

var totalTime = "0:00";
var viewerCount = 7541;
var subCount = 2581;

//  ===================================================== Read + Execute Port Data =================================================================

// Connect to arduino port
arduinoSerialPort.on("open", function () {
  console.log("Serial Port " + arduinoCOMPort + " is opened.");
});

// Listen to incoming commands from arduino and execute them
arduinoSerialPort.on("data", (line) => {
  var newLine = String(line);

  if (newLine.includes("StartStreaming")) {
    obs.sendCallback("StartStreaming", {}, (err, data) => {
      if (data.status === "ok") {
        console.log("Stream is started");
      }
    });
  } else if (newLine.includes("StopStreaming")) {
    obs.sendCallback("StopStreaming", {}, (err, data) => {
      if (data.status === "ok") {
        console.log("Stream is stopped");
      }
    });
  } else if (newLine.includes("SwitchScenes")) {
    handleScenes(newLine);
  } else if (newLine.includes("MicOn")) {
    obs.sendCallback("SetMute", { source: "Mic", mute: false }, (err, data) => {
      if (data.status === "ok") {
        console.log("Mic is unmuted.");
      }
    });
  } else if (newLine.includes("MicOff")) {
    obs.sendCallback("SetMute", { source: "Mic", mute: true }, (err, data) => {
      if (data.status === "ok") {
        console.log("Mic is muted");
      }
    });
  } else if (newLine.includes("CamOn")) {
    obs.sendCallback(
      "SetSceneItemProperties",
      { item: "WebCam", visible: true },
      (err, data) => {
        if (data.status === "ok") {
          console.log("Camera is visible");
        }
      }
    );
  } else if (newLine.includes("CamOff")) {
    obs.sendCallback(
      "SetSceneItemProperties",
      { item: "WebCam", visible: false },
      (err, data) => {
        if (data.status === "ok") {
          console.log("Camera is invisible");
        }
      }
    );
  } else if (newLine.includes("Twitch")) {
    open("https://dashboard.twitch.tv/", { app: "google chrome" });
  } else if (newLine.includes("AudioOn")) {
    audio.muted(false); // set muted
  } else if (newLine.includes("AudioOff")) {
    audio.muted(true); // set muted
  } else if (newLine.includes("Youtube")) {
    open("http://youtube.com", { app: "google chrome" });
  } else if (newLine.includes("Discord")) {
    open("", { app: "discord" });
  } else if (newLine.includes("Obs")) {
    open("", { app: "OBS" });
  }
});

//  ===================================================== Twitch Functions =================================================================
//dq2q28ea9iure3ww2mxbhoaw81qw4m
const clientId = process.env.clientId;
const clientSecret = process.env.clientSecret;
const accessToken = process.env.accessToken;

const twitchClient = TwitchClient.withCredentials(clientId, accessToken, [
  "channel_subscriptions",
]);

// Get Viewer count from Twitch
async function ViewerCounts(userName) {
  const user = await twitchClient.helix.users.getUserByName(userName);
  if (!user) {
    return false;
  }
  return (await twitchClient.helix.streams.getStreamByUserId(user.id)) != null;
}

// Get Subscribers count from Twitch
async function SubCounts(userName) {
  const user = await twitchClient.helix.users.getUserByName(userName);
  if (!user) {
    return false;
  }

  return await twitchClient.kraken.channels.getChannelSubscriptionCount(
    user.id
  );
}

// Update Viewers count
const Viewertimer = new TaskTimer(10000);
Viewertimer.add({
  id: "async-modern",
  callback(task) {
    ViewerCounts("aurateur").then((data) => {
      viewerCount = data.viewers;
      //arduinoSerialPort.write(`${data.viewers},`);
    });
  },
});
//Viewertimer.start();

// Send Subscribers count, Total stream time and Subscribe count to arduino serial port.
const UpdateStateTimer = new TaskTimer(2500);
UpdateStateTimer.add({
  id: "update-state",
  callback(task) {
    arduinoSerialPort.write(` ${totalTime} ${viewerCount} ${subCount} `);
    // console.log(`${totalTime} ${viewerCount} ${subCount}`);
  },
});
UpdateStateTimer.start();

// ===================================================== OBS Functions =================================================================

// connect to OBS Websocket
obs
  .connect({
    address: "localhost:4444",
    password: "1234",
  })
  .then(() => {
    console.log(`Success! We're connected & authenticated.`);
  })
  .catch((err) => {
    // Promise convention dicates you have a catch on every chain.
    console.log(err);
  });

// Update stream time.
obs.on("StreamStatus", (data) => {
  totalTime = fancyTimeFormat(data["total-stream-time"]);
});

// Handler to avoid uncaught exceptions while connecting to OBS Websocket.
obs.on("error", (err) => {
  console.error("socket error:", err);
});

// Function to handle Scene change command from arudino.
function handleScenes(newLine) {
  if (newLine.includes("1")) {
    obs.sendCallback(
      "SetCurrentScene",
      { "scene-name": "Main" },
      (err, data) => {
        console.log(data);
      }
    );
  }
  if (newLine.includes("2")) {
    obs.sendCallback(
      "SetCurrentScene",
      { "scene-name": "Cam" },
      (err, data) => {
        console.log(data);
      }
    );
  }
  if (newLine.includes("3")) {
    obs.sendCallback(
      "SetCurrentScene",
      { "scene-name": "End" },
      (err, data) => {
        console.log(data);
      }
    );
  }
}

// ===================================================== Helper Functions =================================================================

// Convert total time of stream to Hours,Minutes,Seconds
function fancyTimeFormat(time) {
  // Hours, minutes and seconds
  var hrs = ~~(time / 3600);
  var mins = ~~((time % 3600) / 60);
  var secs = ~~time % 60;

  // Output like "1:01" or "4:03:59" or "123:03:59"
  var ret = "";

  if (hrs > 0) {
    ret += "" + hrs + ":" + (mins < 10 ? "0" : "");
  }

  ret += "" + mins + ":" + (secs < 10 ? "0" : "");
  ret += "" + secs;
  return ret;
}

function LineIncludes(line, command) {
  if (line.includes(command)) {
    return line;
  }
}
