# Arduino Deck

Basically an application based on Arduino that allows streamers to design and control, recording of their live streams.

# Problem

Online content producers such as streamers, youtubers, live show hosts need and wants better control schemes for improving accessibility and quality of streams during broadcasting. Currently, they are able to achieve this process with software and hardware tools. But the problem with software part is that: they constantly need keyboard and mouse usage, controlling streaming application without showing it to the audience (we can image this situation as: TV Show’s broadcast company showing broadcast controlling software or terminals to the viewers on the live show, it’s not acceptable by viewers and Live broadcasting system regulations.). On the hardware side: they need extra monitors, Personal computer for controlling broadcasting with video capture device. All these hardware components are not affordable for everyone so they need cheaper alternative software which built on cheap hardware. Such as streaming deck for controlling specific application or operating system components.

# Importance

The main purpose of this project is making cheaper and affordable version of existing solutions which let consumers to control the content and flow of broadcast smoothly with just a piece of hardware. Also it creates convenience for streamers because with a few actions they easily change control of the whole streaming system so it makes them to focus on content and audience more precisely.

# Exsiting Solutions

On the market there are few hardware-based solutions for solving this problem. But the issue with them is that they are extremely expensive which makes hard to reach. As an example, Elgato company selling 3 different types of streaming decks for consumers:

- Elgato 6 Button Stream Deck: 99,000원
- Elgato 15 Button Stream Deck: 209,000원
- Elgato 32 Button Stream Deck: 473,000원

Additionally, for controlling streaming software, the streams can configure key-bindings for each action on the software, but it’s hard to remember, each key-bindings should be unique, must be accessible from every layer of application. And the last solution is using streaming deck applications which are working on monthly subscription and limiting usage of mobile phone continuously.

# Solution

Essentially, my initial solution approach is this: Creating Fully customizable LCD Display Deck on Arduino Mega with 15 actions button support for controlling OBS (Open Broadcast Software) and operating system components (volume, open application, control brightness and etc.) with live stream information bar.
![Arudino Deck Inital Design](https://i.imgur.com/2oq5zjO.png)
For implementing these solution, we need nearly 20,000 - 30,000 Korean Won (Arduino Mega (8,000원) + TFT LCD Touch Screen 480x 320 (16,000원) + SD card). Basically, this solution is cheaper 70% cheaper than low-cost model of Arduino streaming deck.

# Execution

Execution process divided into 2 parts:

- **Hardware Requirements:**

  - ARDUINO MEGA 2560 (Higher Flash memory than Uno)
  - TFT LCD Touch Screen Display Shield (480x320) - ILI9486
  - USB 2.0 CABLE TYPE A/B
  - Micro SD Card (Any size is preferable)

- **Software & Libraries**
  - **Server Side (NodeJS)**
    - **@Serial-port**: Node.js package to access serial ports for Linux, OSX and Windows.
    - **@OBSWebSocket**: WebSocket API for controlling actions, settings, broadcast schemes of OBS Studio.
    - **@TwitchClientAPI**: Library for interacting with Twitch's API, chat, PubSub and subscribe to Webhooks.
  - **Arduino Deck**
    - MCUFRIEND_kbv: Based on Adafruit_GFX Library which allows us Drawing Shapes, Images, Texts on Display. Also, it includes TouchScreen Library for controlling deck.
    - SD & SPI (Serial Peripheral Interface): Micro SD library for display bitmap image.
    - Arduino Standard Library
  - **Obs-websocket** plugin for OBS application: A Remote-Control Websocket API for OBS Studio. The websocket server runs on port 4444 and the protocol is based on the OBSRemote protocol (including authentication) with some additions specific to OBS Studio.

![Workflow Graph](https://i.imgur.com/W6sO45P.png)

**I used Platform.IO and Visual Studio Code IDE so project hierarchy based on Platform.IO standards. If you want to execute this code on your hardware please first install Visual Studio Code and then Platform.IO plugin.**

File System of Project:

- **Index.js**: NojdeJS application for starting server-side connection with Arduino deck and it is constantly emitting and receiving commands and information from / to Arduino deck. The connection with the deck established by serial ports.

  - **Working Scheme**:
  - 1.  Listen for incoming execution command-> Read execution command -> If Command is valid -> Execute command and emit result of command.
  - 2.  If streaming is live -> Emit details of live stream (streaming time, viewers count, subscribers count)

- **Src/Main.ino** : Hardware source file for controlling Arduino Mega and TFT LCD screen. It also constantly emitting and receiving commands and information from Serial Port. Handling User Interface of Deck, Touch events, Loading / Drawing images from Micro SD card.

  - **Working Scheme**:
    - 1. Listen for incoming streaming info -> Read info -> Parse Info -> Update interface with processed information
    - 2. Commands are indexed with button order. Let’s say we have 2 buttons and 2 commands: First Button, Second Button & FirstCommand and Second Command which means First Button pointing to FirstCommand, Second Button pointing to SecondCommand (ButtonFirst -> FirstCommand, ButtonSecond -> SecondCommand) So when a user touches the button Arduino deck emitting name of command to the serial port which enumerated with number of indexed button.

- **Lib folder**: Contains used libraries in this project.
- **Node_modules**: Package folder of NodeJS
