/*
  asdfasdf

   ESP32 FastLED WebServer: https://github.com/jasoncoon/esp32-fastled-webserver
   Copyright (C) 2017 Jason Coon

   Built upon the amazing FastLED work of Daniel Garcia and Mark Kriegsman:
   https://github.com/FastLED/FastLED

   ESP32 support provided by the hard work of Sam Guyer:
   https://github.com/samguyer/FastLED

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0

#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <FastLED_NeoMatrix.h>
#include <LEDMatrix.h>

//rgb bitmap 
#include "bild.h"

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001008)
#warning "Requires FastLED 3.1.8 or later; check github for latest code."
#endif

AsyncWebServer webServer(80);
WebSocketsServer webSocketsServer = WebSocketsServer(81);

const int led = 32;
const int LED_BUILTIN = 2;

uint8_t autoplay = 1;
uint8_t autoplayDuration = 60;
unsigned long autoPlayTimeout = 0;
uint8_t currentPatternIndex = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t power = 1;
uint8_t brightness = 150;
uint8_t speed = 20;

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
uint8_t cooling = 50;

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
uint8_t sparking = 120;

CRGB solidColor = CRGB::Blue;

uint8_t cyclePalettes = 0;
uint8_t paletteDuration = 10;
uint8_t currentPaletteIndex = 0;
unsigned long paletteTimeout = 0;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define DATA_PIN 14 // pins tested so far on the Feather ESP32: 13, 12, 27, 33, 15, 32, 14, SCL
//#define CLK_PIN   4
#define LED_TYPE WS2811
#define COLOR_ORDER GRB //GRB
#define NUM_STRIPS 1
#define NUM_LEDS_PER_STRIP 30
#define NUM_LEDS NUM_LEDS_PER_STRIP *NUM_STRIPS
#define MILLI_AMPS 1000 // IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
#define FRAMES_PER_SECOND 120

// -- NeoMatrix configs
#define MATRIX_TILE_WIDTH   6 // width of EACH NEOPIXEL MATRIX (not total display)
#define MATRIX_TILE_HEIGHT  5 // height of each matrix
#define MATRIX_TILE_H       1  // number of matrices arranged horizontally
#define MATRIX_TILE_V       1  // number of matrices arranged vertically
#define mw (MATRIX_TILE_WIDTH *  MATRIX_TILE_H)
#define mh (MATRIX_TILE_HEIGHT * MATRIX_TILE_V)
#define NUMMATRIX (mw*mh)
#define LED_GREEN_HIGH 		(63 << 5)
#define LED_BLACK		0

cLEDMatrix<-MATRIX_TILE_WIDTH, -MATRIX_TILE_HEIGHT, HORIZONTAL_ZIGZAG_MATRIX,
    MATRIX_TILE_H, MATRIX_TILE_V, HORIZONTAL_BLOCKS> ledmatrix;

//CRGB leds[NUM_LEDS]; //server version 
CRGB *leds = ledmatrix[0];
/*
   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
    Position of the FIRST LED in the FIRST MATRIX; pick two, e.g.
    NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
  NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs WITHIN EACH MATRIX are
    arranged in horizontal rows or in vertical columns, respectively;
    pick one or the other.
  NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns WITHIN
    EACH MATRIX proceed in the same order, or alternate lines reverse
    direction; pick one.
  NEO_TILE_TOP, NEO_TILE_BOTTOM, NEO_TILE_LEFT, NEO_TILE_RIGHT:
    Position of the FIRST MATRIX (tile) in the OVERALL DISPLAY; pick
    two, e.g. NEO_TILE_TOP + NEO_TILE_LEFT for the top-left corner.
  NEO_TILE_ROWS, NEO_TILE_COLUMNS: the matrices in the OVERALL DISPLAY
    are arranged in horizontal rows or in vertical columns, respectively;
    pick one or the other.
  NEO_TILE_PROGRESSIVE, NEO_TILE_ZIGZAG: the ROWS/COLUMS OF MATRICES
    (tiles) in the OVERALL DISPLAY proceed in the same order for every
    line, or alternate lines reverse direction; pick one.  When using
    zig-zag order, the orientation of the matrices in alternate rows
    will be rotated 180 degrees (this is normal -- simplifies wiring).
    */
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, MATRIX_TILE_WIDTH, MATRIX_TILE_HEIGHT,
MATRIX_TILE_H, MATRIX_TILE_V,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
    NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG +
    NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_PROGRESSIVE);


// -- The core to run FastLED.show()
#define FASTLED_SHOW_CORE 0

#include "patterns.h"

#include "field.h"
#include "fields.h"

#include "secrets.h"
#include "wifi_local.h"
#include "web.h"
/*
  wifi ssid and password should be added to a file in the sketch named secrets.h
  the secrets.h file should be added to the .gitignore file and never committed or
  pushed to public source control (GitHub).
  const char* ssid = "........";
  const char* password = "........";
*/

#include "GifDecoder.hpp"



// -- Task handles for use in the notifications
static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t userTaskHandle = 0;


// --- Gif Stuff taken from SimpleGifAnimViewer.ino 
File file;
const char *pathname = "/gifs/filename.gif";
GifDecoder<mw, mh, 11> decoder;


bool fileSeekCallback(unsigned long position) { return file.seek(position); }
unsigned long filePositionCallback(void) { return file.position(); }
int fileReadCallback(void) { return file.read(); }
int fileReadBlockCallback(void * buffer, int numberOfBytes) { return file.read((uint8_t*)buffer, numberOfBytes); }
void screenClearCallback(void) { matrix->clear(); }
void updateScreenCallback(void) { matrix->show(); }
void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
  CRGB color = CRGB(matrix->gamma[red], matrix->gamma[green], matrix->gamma[blue]);
  //if you want to display gifs bigger than matrix size, look into example again, theres a bouncing way with offsets
  matrix->drawPixel(x, y, color);
}
// --- Gif Stuff




void matrix_clear() {
    // FastLED.clear does not work properly with multiple matrices connected via parallel inputs
    memset(leds, 0, NUMMATRIX*3);
}

/** show() for ESP32
    Call this function instead of FastLED.show(). It signals core 0 to issue a show,
    then waits for a notification that it is done.
*/
void FastLEDshowESP32()
{
  if (userTaskHandle == 0)
  {
    // -- Store the handle of the current task, so that the show task can
    //    notify it when it's done
    userTaskHandle = xTaskGetCurrentTaskHandle();

    // -- Trigger the show task
    xTaskNotifyGive(FastLEDshowTaskHandle);

    // -- Wait to be notified that it's done
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200);
    ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    userTaskHandle = 0;
  }
}

/** show Task
    This function runs on core 0 and just waits for requests to call FastLED.show()
*/
void FastLEDshowTask(void *pvParameters)
{
  // -- Run forever...
  for (;;)
  {
    // -- Wait for the trigger
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // -- Do the show (synchronously)
    FastLED.show();

    // -- Notify the calling task
    xTaskNotifyGive(userTaskHandle);
  }
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  currentPatternIndex = (currentPatternIndex + 1) % patternCount;

  String json = "{\"name\":\"pattern\",\"value\":\"" + String(currentPatternIndex) + "\"}";
  webSocketsServer.broadcastTXT(json);
}

void nextPalette()
{
  currentPaletteIndex = (currentPaletteIndex + 1) % paletteCount;
  targetPalette = palettes[currentPaletteIndex];

  String json = "{\"name\":\"palette\",\"value\":\"" + String(currentPaletteIndex) + "\"}";
  webSocketsServer.broadcastTXT(json);
}

//displays text which scrolls through screen
void display_scrollText(String text) {

    //calculates font size according to mw(matrix_width)
    uint8_t size = max(int(mw/8), 1);
    matrix->clear();
    matrix->setTextWrap(false);  // we don't wrap text so it scrolls nicely
    matrix->setTextSize(size);
    matrix->setRotation(0);  //2 +90 degrees;3 -90 degres; 0 default;1 -180 degrees
    for (int8_t x=6; x>=-42; x--) {
        //used to be: x=7, x>=-42, x--
	    yield();
	    matrix->clear();
	    matrix->setCursor(x,0);
	    matrix->setTextColor(LED_GREEN_HIGH);
	    matrix->print(text);
	    matrix->show();
       delay(150); //controls tim intervall between iteration's (how fast text scrolls)
    }
}

//generates a well formed bitmaps for Adafruits GFX Backend 
void fixdrawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h) {
/*
Laut Beispiel ist die Bitmap in bild.h nicht komaptibel mit Adafruit_gfx Biblio, deshalb muss sie konvertiert werden
Es handelt sich um eine Konversion von BRG 4 bit Pixelmap zu ein er 5/6/5 RGB Bitmap, welche von Adafruit_gfx genutzt wird
https://github.com/marcmerlin/FastLED_NeoMatrix/blob/master/examples/MatrixGFXDemo/MatrixGFXDemo.ino line 325
*/
    uint16_t RGB_bmp_fixed[w * h];
    for (uint16_t pixel=0; pixel<w*h; pixel++) {
	    uint8_t r,g,b;
	    uint16_t color = pgm_read_word(bitmap + pixel);

	    b = (color & 0xF00) >> 8;
	    g = (color & 0x0F0) >> 4;
	    r = color & 0x00F;

	    // expand from 4/4/4 bits per color to 5/6/5
    	b = map(b, 0, 15, 0, 31);
    	g = map(g, 0, 15, 0, 63);
    	r = map(r, 0, 15, 0, 31);

	    RGB_bmp_fixed[pixel] = (r << 11) + (g << 5) + b;

    }
    matrix->drawRGBBitmap(x, y, RGB_bmp_fixed, w, h);
    matrix->show();
}

//displays Bitmaps, after colors and bits per color has been corrected
void display_rgbBitmap() { 
    static uint16_t bmx,bmy;
    Serial.println("rgb bitmaps started");
    //clear all Leds
    matrix->fillRect(bmx,bmy, bmx+8,bmy+8, LED_BLACK);
    fixdrawRGBBitmap(0, 0, bitmap, 6, 5);
     Serial.println("went through with success");
}

void setup()
{
  delay(5000);
  pinMode(led, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(led, 1);

  Serial.begin(9600);

  SPIFFS.begin(); //true, worked without errors
  listDir(SPIFFS, "/", 1);

  // restore from memory
  loadFieldsFromEEPROM(fields, fieldCount);
  
  setupWifi();
  setupWeb();

  //FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<NEOPIXEL,DATA_PIN>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip); //server version
  FastLED.addLeds<NEOPIXEL,DATA_PIN>(leds, NUMMATRIX).setCorrection(TypicalLEDStrip);
  Serial.print("Matrix Size: ");
  Serial.print(mw);
  Serial.print(" ");
  Serial.println(mh);
  matrix->begin();
  matrix->setTextWrap(false);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);

  //set master brightness control
  FastLED.setBrightness(brightness);

  int core = xPortGetCoreID();
  Serial.print("Main code running on core ");
  Serial.println(core);

  // -- Create the FastLED show task
  xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, NULL, 2, &FastLEDshowTaskHandle, FASTLED_SHOW_CORE);

  autoPlayTimeout = millis() + (autoplayDuration * 1000);

  //--- Gif Stuff happening here
  // error gets thrown when calling functions of decoder class;;;FIND OUT WHY;;;;
 decoder.setScreenClearCallback(screenClearCallback);
  // decoder.setUpdateScreenCallback(updateScreenCallback);
  // decoder.setDrawPixelCallback(drawPixelCallback);

  // decoder.setFileSeekCallback(fileSeekCallback);
  // decoder.setFilePositionCallback(filePositionCallback);
  // decoder.setFileReadCallback(fileReadCallback);
  // decoder.setFileReadBlockCallback(fileReadBlockCallback);

  // file = SPIFFS.open(pathname, "r");
  //   if (!file) {
  //     Serial.print("Error opening GIF file ");
  //     Serial.println(pathname);
	//     while (1) { delay(1000); }; // while 1 loop only triggers watchdog on ESP chips
  //   }
  //   decoder.startDecoding();

  // //--- Gif Stuff ending here
}

void loop()
{
  handleWeb();

  if (power == 0)
  {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }
  else
  {
    // display_scrollText("123");
    display_rgbBitmap();
    delay(1000);
    /*
      How will these functions will be called from outside?

      3 Major functionalities will be implemented
      Text 
      Bitmap 
      gif 

      ---- Text:
      -[x] display_scrollText(String txt); wants texts which will be immeadiatly displayed on matrix
      -[ ] post method();  
      -[ ] display_time() external call, that listens on time and changes it value

      ---- Bitmap:
      -[x] display_bitmap
      -[ ] upload bitmap via http POST

      ---- GIF:
      -[ ] display gif on matrix am
      -[ ] upload Gif via http POST



    */

   // --- Gif Stuff
  // decoder.decodeFrame();
   // --- Gif Stuff
  }
  
  FastLED.delay(1000 / FRAMES_PER_SECOND);
  Serial.print("End of Loop, starting again...");
}
