#include <Arduino.h>
#include <pin_magic.h>
#include <registers.h>
#include <Adafruit_GFX.h>
#include <TouchScreen.h>
#include <SPI.h>
#include <stdio.h>
#include <string.h>
#include <MCUFRIEND_kbv.h>
#include <stdint.h>
#include <SD.h>
#include <Fonts/FreeSans12pt7b.h>
MCUFRIEND_kbv tft;

#define USE_SD_CARD

#define YP A3 // must be an analog pin, using "A" notation
#define XM A2 // must be an analog pin, using "A" notation
#define YM 9  // Adigital pin
#define XP 8  // A digital pin

// Calibration X Y for raw data when touching edges of screen
#define Bottom 130
#define Right 85
#define Top 890
#define Left 910

//SPI Communication
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

//Color Definitons
#define BLACK 0x0000
#define TwitchPurple tft.color565(100, 65, 165)
#define WHITE 0xFFFF

#define MINPRESSURE 1
#define MAXPRESSURE 1000

//SD Card pin
#define SD_CS 10

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
// Pins A2-A6
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 364);

//Size of key containers 80px
#define BOXSIZE 80
#define BoxWidth 70
#define BoxHeight 70

//Variables for touch coordinates
int X, Y, Z;

//Space between squares
double padding = 6;
double horizontalPadding = 20;
double buttonEdge = 3;
double edge = 3;
double info = padding + BOXSIZE * 2 / 3 + padding;

// Columns of buttons
double C1 = buttonEdge;
double C2 = C1 + BOXSIZE + horizontalPadding;
double C3 = C2 + BOXSIZE + horizontalPadding;
double C4 = C3 + BOXSIZE + horizontalPadding;
double C5 = C4 + BOXSIZE + horizontalPadding + 3;

// Row of buttons
double R1 = info + edge + padding;
double R2 = R1 + BOXSIZE + padding;
double R3 = R2 + BOXSIZE + padding;

// Array of Box Layout
double col[] = {C1, C2, C3, C4, C5};
double row[] = {R1, R2, R3};

// Buttons
Adafruit_GFX_Button first, second, third, fourth, fifth, sixth, seventh, eighth, ninth, tenth, eleventh, twelveth, thirteenth, fourteenth, fifteenth;
Adafruit_GFX_Button *buttons[] = {&first, &second, &third, &fourth, &fifth, &sixth, &seventh, &eighth, &ninth, &tenth, &eleventh, &twelveth, &thirteenth, &fourteenth, &fifteenth, NULL};

// Image File Names in SD Card folder.
String fileNames[] = {"On.bmp", "Off.bmp", "Sce1.bmp", "Sce2.bmp", "Sce3.bmp", "MicOn.bmp", "MicOff.bmp", "CamOn.bmp", "CamOff.bmp", "Twitch.bmp", "AuOn.bmp", "AuOff.bmp", "You.bmp", "Dis.bmp", "OBS.bmp"};
// Command Names
char *command[] = {"StartStreaming", "StopStreaming", "SwitchScenes-1", "SwitchScenes-2", "SwitchScenes-3", "MicOn", "MicOff", "CamOn", "CamOff", "Twitch", "AudioOn", "AudioOff", "Youtube", "Discord", "Obs"};

void setup()
{
  // Start Serial
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println(info);

  // id of the TFT lcd screen
  uint16_t identifier = tft.readID();

  // start screen
  tft.begin(identifier);

  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(SD_CS))
  {
    Serial.println(F("Failed to intializing sd card"));
    return;
  }
  //Make Screen Landscape orientation
  tft.setRotation(1);

  //Background color
  tft.fillScreen(BLACK);

  // draw buttons
  drawButtons();

  // Draw Icons
  drawButtonImages();

  // draw User Interface
  drawBoxes();
}

void loop()
{
  update_button_list(buttons); //use helper function

  // Check If Buttons are pressed, Emit Command from Pressed Button.
  for (int i = 0; buttons[i] != NULL; i++)
  {
    if (buttons[i]->justPressed())
    {
      // Serial.print(fileNames[i]);
      // Serial.println(" Just Pressed!");

      if (Serial.availableForWrite())
      {
        Serial.println(command[i]);
      }
    }
  }
  fetchInfo();
}

// Constantly checking if user pressed to touchscreen and mapping coordinates of the touch to X Y Z coordinates.
bool retrieveTouch(void)
{
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  //If sharing pins, I need to fix the directions of the touchscreen pins
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  // on my tft the numbers are reversed so swaped X and Y.
  X = map(p.y, Left, Right, 0, tft.width());
  Y = map(p.x, Top, Bottom, 0, tft.height());
  Z = p.z;

  bool pressed = (Z > MINPRESSURE && Z < MAXPRESSURE);

  if (Z > MINPRESSURE && Z < MAXPRESSURE)
  {
    // Serial.print("X = ");
    // Serial.print(X);
    // Serial.print("\tY = ");
    // Serial.print(Y);
    // Serial.print("\tPressure = ");
    // Serial.println(Z);
  }
  return pressed;
}

// Checking if a Button Pressed by mapping touch coordinates with button coordinates.
bool updateButton(Adafruit_GFX_Button *b, bool down)
{
  b->press(down && b->contains(X, Y));

  return down;
}

// Check each button is pressed or not.
bool update_button_list(Adafruit_GFX_Button **pb)
{
  bool down = retrieveTouch();
  for (int i = 0; buttons[i] != NULL; i++)
  {
    updateButton(buttons[i], down);
  }
  return down;
}

// Draw User Interface boxes.
void drawBoxes()
{
  for (int j = 0; j < 3; j++)
  {
    for (int i = 0; i < 5; i++)
    {
      tft.drawRect(col[i], row[j], BoxWidth, BoxHeight, TwitchPurple);
    }
  }

  tft.drawRect(edge, edge, tft.width() - padding, info - edge, WHITE);
}
// Draw Buttons in boxes.
void drawButtons()
{
  int count = 0;
  for (int j = 0; j < 3; j++)
  {
    for (int i = 0; i < 5; i++)
    {
      buttons[count]->initButton(&tft, col[i] + 35, row[j] + 35, BoxWidth - 5, BoxHeight - 5, BLACK, BLACK, BLACK, "", 1);
      buttons[count]->drawButton(1);
      count++;
    }
  }
}

char fileName[100];
// Draw Each Image from SD Card to the screen.
void drawButtonImages()
{
  int count = 0;
  for (int j = 0; j < 3; j++)
  {
    for (int i = 0; i < 5; i++)
    {
      fileNames[count].toCharArray(fileName, 100);
      bmpDraw(fileName, col[i] + 2, row[j] + 2);
      count++;
    }
  }
}

// Fetch incoming information about Stream time,Subscriber and Viewers count.
void fetchInfo()
{
  DrawConstantString(315, 40, 1, &FreeSans12pt7b, "Time:", TwitchPurple);
  DrawConstantString(10, 40, 1, &FreeSans12pt7b, "Subs:", TwitchPurple);
  tft.drawLine(135, 3, 135, 64, TwitchPurple);
  DrawConstantString(145, 40, 1, &FreeSans12pt7b, "Viewers:", TwitchPurple);
  tft.drawLine(305, 3, 305, 64, TwitchPurple);

  if (Serial.available())
  {
    String state = Serial.readString();

    char totalTime[256];
    char viewersCount[256];
    char subsCount[256];

    // Assign each state to the variables.
    splitState(state, totalTime, viewersCount, subsCount);

    // Stream Time
    DrawVariableString(378, 41, 1, &FreeSans12pt7b, String(totalTime), WHITE);

    // Subs Count Text
    DrawVariableString(75, 41, 1, &FreeSans12pt7b, String(subsCount), WHITE);

    // Viewers Count Text
    DrawVariableString(240, 41, 1, &FreeSans12pt7b, String(viewersCount), WHITE);
  }
}

// Writing a String to Display
void DrawConstantString(int x, int y, int sz, const GFXfont *f, const String msg, const uint16_t color)
{
  int16_t x1, y1;
  uint16_t wid, ht;
  tft.setFont(f);
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(sz);
  tft.println(msg);
}

// Current graphic library don't support re-drawing of strings so we should cover previous string with black pixels and overwrite it with new string.
void DrawVariableString(int x, int y, int sz, const GFXfont *f, const String msg, const uint16_t color)
{
  // variables of storing x,y,width,height of string.
  int16_t x1, y1;
  uint16_t wid, ht;
  // function for get text bounds of string.
  tft.getTextBounds(msg, x, y, &x1, &y1, &wid, &ht);
  // filling bounds of previous string.
  tft.fillRect(x1 - 5, y1 - 3, wid + 15, ht + 5, BLACK);

  tft.setFont(f);
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(sz);
  tft.println(msg);
}

// Function to split State string into three parts: time, subs, viewers.
void splitState(const String state, char totalTime[], char viewersCount[], char subsCount[])
{
  char str[1024];
  int count = 0;
  state.toCharArray(str, sizeof(str));
  int init_size = strlen(str);
  char delim[] = " ";

  char *ptr = strtok(str, delim);

  while (ptr != NULL)
  {
    switch (count)
    {
    case 0:
      strcpy(totalTime, ptr);
      break;
    case 1:
      strcpy(viewersCount, ptr);
      break;
    case 2:
      strcpy(subsCount, ptr);
      break;
    }

    ptr = strtok(NULL, delim);
    count++;
  }
}

// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.
#define BUFFPIXEL 60

void bmpDraw(char *filename, int x, int y)
{

  File bmpFile;
  int bmpWidth, bmpHeight;            // W+H in pixels
  uint8_t bmpDepth;                   // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;            // Start of image data in file
  uint32_t rowSize;                   // Not always = bmpWidth; may have padding
  uint8_t sdbuffer[3 * BUFFPIXEL];    // pixel in buffer (R+G+B per pixel)
  uint16_t lcdbuffer[BUFFPIXEL];      // pixel out buffer (16-bit per pixel)
  uint8_t buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean goodBmp = false;            // Set to true on valid header parse
  boolean flip = true;                // BMP is stored bottom-to-top
  int w, h, row, col;
  uint8_t r, g, b;
  uint32_t pos = 0, startTime = millis();
  uint8_t lcdidx = 0;
  boolean first = true;

  if ((x >= tft.width()) || (y >= tft.height()))
    return;

  //Serial.println();
  //Serial.print(F("Loading image '"));
  //Serial.print(filename);
  //Serial.println('\'');
  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL)
  {
    //Serial.println(F("File not found"));
    return;
  }

  // Parse BMP header
  if (read16(bmpFile) == 0x4D42)
  { // BMP signature
    //Serial.println(F("File size: "));
    read32(bmpFile);
    (void)read32(bmpFile);            // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    //Serial.print(F("Image Offset: "));
    (bmpImageoffset, DEC);
    // Read DIB header
    //Serial.print(F("Header size: "));
    read32(bmpFile);
    bmpWidth = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if (read16(bmpFile) == 1)
    {                             // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      //Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if ((bmpDepth == 24) && (read32(bmpFile) == 0))
      { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        //Serial.print(F("Image size: "));
        //Serial.print(bmpWidth);
        //Serial.print('x');
        //Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if (bmpHeight < 0)
        {
          bmpHeight = -bmpHeight;
          flip = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if ((x + w - 1) >= tft.width())
          w = tft.width() - x;
        if ((y + h - 1) >= tft.height())
          h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x + w - 1, y + h - 1);

        for (row = 0; row < h; row++)
        { // For each scanline...
          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if (bmpFile.position() != pos)
          { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col = 0; col < w; col++)
          { // For each column...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer))
            {
              // Push LCD buffer to the display first
              if (lcdidx > 0)
              {
                tft.pushColors(lcdbuffer, lcdidx, first);
                lcdidx = 0;
                first = false;
              }
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            lcdbuffer[lcdidx++] = tft.color565(r, g, b);
          } // end pixel
        }   // end scanline
        // Write any remaining data to LCD
        if (lcdidx > 0)
        {
          tft.pushColors(lcdbuffer, lcdidx, first);
        }
        //Serial.print(F("Loaded in "));
        //Serial.print(millis() - startTime);
        //Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if (!goodBmp)
    Serial.println(F("BMP format not recognized."));
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f)
{
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f)
{
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}