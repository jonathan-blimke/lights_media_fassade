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
// -- The core to run FastLED.show()
#define FASTLED_SHOW_CORE 0


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


AsyncWebServer webServer(80);
WebSocketsServer webSocketsServer = WebSocketsServer(81);

const int led = 32;
const int LED_BUILTIN = 2;


uint8_t power = 1;
String serverstring = "hello world";
uint8_t brightness = 150;
uint16_t bitmap[30] = {
0xFFFF, 0x000F, 0x00F0, 0x0F00, 0xF000, 0xAB10,     
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000F,  
};
String array ="arraymaincpp";



#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define DATA_PIN 14 
#define LED_TYPE WS2811
#define COLOR_ORDER RGB //GRB
#define NUM_STRIPS 1
#define NUM_LEDS_PER_STRIP 30
#define NUM_LEDS NUM_LEDS_PER_STRIP *NUM_STRIPS
#define MILLI_AMPS 1000 //IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
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

// CRGB leds[NUM_LEDS]; //server version 
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

#include "field.h"
#include "fields.h"
#include "secrets.h"
#include "wifi_local.h"
#include "web.h"
#include "GifDecoder.h"

// -- Task handles for use in the notifications
static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t userTaskHandle = 0;

File file;
int OFFSETX =  -1;
int OFFSETY =  0;
const char *pathname = "/gif.gif";
GifDecoder<mw, mh, 11> decoder;

bool fileSeekCallback(unsigned long position) { return file.seek(position); }
unsigned long filePositionCallback(void) { return file.position(); }
int fileReadCallback(void) { return file.read(); }
int fileReadBlockCallback(void * buffer, int numberOfBytes) { return file.read((uint8_t*)buffer, numberOfBytes); }
void screenClearCallback(void) { matrix->clear(); }
void updateScreenCallback(void) { matrix->show(); }
void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
    CRGB color = CRGB(matrix->gamma[red], matrix->gamma[green], matrix->gamma[blue]);
    // matrix->drawPixel(x, y, color);
     matrix->drawPixel(x+OFFSETX, y+OFFSETY, color);
  } 
// --- Gif Stuff

void makeFileonSpiffs() {
  
   File file = SPIFFS.open("/test.txt", FILE_WRITE);
    if (!file) {
      Serial.println("There was an error opening the file for writing");
      return;
    }
    if (file.print("TEST")) {
      Serial.println("File was written");
    } else {
      Serial.println("File write failed");
    }
}



void printArray(int* array) {
  int b;
  for (b = 0; b < NUM_LEDS; b++) {
    Serial.print(array[b]);
    Serial.print(",");
  }
}

uint16_t* convertStrtoArr(String str) { 
    int str_length = NUM_LEDS; 
    uint16_t arr[str_length] = { 0 }; 
  
    int j = 0, i;
  
    for (i = 1; i <  array.length()-1 ; i++) { 
        // if str[i] is ', ' then split 
        if (str[i] == ',') { 
            // Increment j to point to next array index
            j++; 
        } 
        else { 
            // subtract str[i] by 48 to convert it to int 
            // generate dezimal number by *10  
            arr[j] = arr[j] * 10 + (str[i] - 48);
        } 
    }
  // printArray(arr);
  return arr;
} 

void matrix_clear() {
    // FastLED.clear does not work properly with multiple matrices connected via parallel inputs
    memset(leds, 0, NUMMATRIX*3);
}

/** show() for ESP32
    Call this function instead of FastLED.show(). It signals core 0 to issue a show,
    then waits for a notification that it is done.
*/
void FastLEDshowESP32() {
  if (userTaskHandle == 0) {
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
void FastLEDshowTask(void *pvParameters) {
  // -- Run forever...
  for (;;) {
    // -- Wait for the trigger
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // -- Do the show (synchronously)
    FastLED.show();

    // -- Notify the calling task
    xTaskNotifyGive(userTaskHandle);
  }
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

//displays text which scrolls through screen
void display_scrollText(String text) {

    //calculates font size according to mw(matrix_width)
    uint8_t size = max(int(mw/8), 1);
    matrix->clear();
    matrix->setTextWrap(false);  //stops text from standing still
    matrix->setTextSize(size);
    matrix->setRotation(0);  //2 +90 degrees;3 -90 degres; 0 default;1 -180 degrees
    int8_t length = serverstring.length();
    Serial.println(length);
    int16_t looplength = length * mw;
    Serial.println(looplength);
    for (int8_t x=0; x>=-looplength; x--) {
        //used to be: x=7, x>=-42, x--
	    yield();
	    matrix->clear();
	    matrix->setCursor(x,0);
	    matrix->setTextColor(LED_GREEN_HIGH);
	    matrix->print(text);
	    matrix->show();
       delay(100); //controls tim intervall between iteration's (how fast text scrolls)
    }
}

//generates a well formed bitmaps for Adafruits GFX Backend Konversion von BRG 4 bit Pixelmap zu ein er 5/6/5 RGB Bitmap (https://github.com/marcmerlin/FastLED_NeoMatrix/blob/master/examples/MatrixGFXDemo/MatrixGFXDemo.ino) line 325
void fixdrawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h) {

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
void display_rgbBitmap(uint16_t* bitmapinput) { 
    static uint16_t bmx,bmy;
    //clears all Leds could also try matrix_clear();
    matrix->fillRect(bmx,bmy, bmx+8,bmy+8, LED_BLACK);
    fixdrawRGBBitmap(0, 0, bitmapinput, 6, 5);
}

void display_gif() {
    matrix->clear();
    decoder.decodeFrame();
    //clear?
}

void bitmapsIterationTest() {

  uint16_t bitmap0[30] = {
  0xFFFF, 0xc330, 0x333c, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  };

  uint16_t bitmap1[30] = {
  0x0000, 0xFFFF, 0xc330, 0x333c, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  };

  uint16_t bitmap2[30] = {
  0x0000, 0x0000, 0xFFFF, 0xc330, 0x333c, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  };


  uint16_t bitmap3[30] = {
  0x0000, 0x0000, 0x0000, 0xFFFF, 0xc330, 0x333c,    
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  };

  uint16_t bitmap4[30] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  };

  uint16_t bitmap5[30] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF,   
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  };

  uint16_t bitmap6[30] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
  0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  };

  uint16_t bitmap7[30] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  };

  uint16_t bitmap8[30] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0xFFFF, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  };

  uint16_t bitmap9[30] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  0x0000, 0x0000, 0x0000, 0xFFFF, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  };

  uint16_t bitmap10[30] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  };

  uint16_t bitmap11[30] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  
  };

  //______________
  int delayTime = 100;

  display_rgbBitmap(bitmap0);
  delay(delayTime);
  //___
  display_rgbBitmap(bitmap1);
  delay(delayTime);
  //___
  display_rgbBitmap(bitmap2);
  delay(delayTime);
  //___
  display_rgbBitmap(bitmap3);
  delay(delayTime);
  //___
  display_rgbBitmap(bitmap4);
  delay(delayTime);
  //___
  display_rgbBitmap(bitmap5);
  delay(delayTime);
  //___
  display_rgbBitmap(bitmap6);
  delay(delayTime);
  //___
  display_rgbBitmap(bitmap7);
  delay(delayTime);
  //___
  display_rgbBitmap(bitmap8);
  delay(delayTime);
  //___
  display_rgbBitmap(bitmap9);
  delay(delayTime);
  //___
  display_rgbBitmap(bitmap10);
  delay(delayTime);
  //___
  display_rgbBitmap(bitmap11);
  delay(delayTime);
 
}

void setup() {
  delay(5000);
  pinMode(led, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(led, 1);

  Serial.begin(9600);

  SPIFFS.begin(); 
  listDir(SPIFFS, "/", 1); //lists SPIFF directory in Serial Monitor String parameter defines path

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

  //--- Gif Stuff happening here
  decoder.setScreenClearCallback(screenClearCallback);
  decoder.setUpdateScreenCallback(updateScreenCallback);
  decoder.setDrawPixelCallback(drawPixelCallback);
  decoder.setFileSeekCallback(fileSeekCallback);
  decoder.setFilePositionCallback(filePositionCallback);
  decoder.setFileReadCallback(fileReadCallback);
  decoder.setFileReadBlockCallback(fileReadBlockCallback);

  file = SPIFFS.open(pathname, "r");
    if (!file) {
      Serial.print("Error opening GIF file ");
      Serial.println(pathname);
	    while (1) { delay(1000); }; // 1 while 1 loop only triggers watchdog on ESP chips
    }
    decoder.startDecoding();
  //--- Gif Stuff ending here

}

void loop(){
  handleWeb();

  if (power == 0) 
  {
    // fill_solid(leds, NUM_LEDS, CRGB::Black);
    bitmapsIterationTest();
  }
  else
  {
    // display_scrollText(serverstring);
    // display_rgbBitmap(bitmap);
    // Serial.print("array = ");
    // Serial.print(array);
    // convertStrtoArr(array);
    
  
    // matrix_clear();
    display_gif();
    // decoder.decodeFrame();
    // delay(5000);
    // makeFileonSpiffs();
    // listDir(SPIFFS, "/", 0);
    // display_rgbBitmap(convertStrtoArr(array));
    /*
      How will these functions will be called from outside?

      3 Major functionalities will be implemented
      Text 
      Bitmap 
      gif 

      ---- Text:
      -[x] display_scrollText(String txt); wants texts which will be immeadiatly displayed on matrix
      -[x] server post method();  
      -[x] client post method(); 
      -[x] frontend gui, textinput, fileupload(gif, bitmap)
  
      ---- Bitmap:
      -[x] display_bitmap
      -[x] save bitmap into json backend as String
      -[x] serializing it as an array (hex array -> string -> int array)
      -[x] iterieren über bitmaps?
      -[ ] complete cycle pars int into 4 bit hex values
      -[ ] upload bitmap via http POST
      -[ ] parse them into 4 bit hex value

      ---- GIF:
      -[x] import marcmerlin gif decoder
      -[x] display gif from spiff on matrix 
      -[ ] upload_Gif() via http POST into in app.js
      -[ ] save it into SPIFF

    */
  }
  // delay(1000);
  
  // FastLED.delay( 1000 / FRAMES_PER_SECOND);
  Serial.print("End of Loop, starting again..."); 
}
