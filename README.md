# Welcome


This is a project currently work in progress is a ESP32 Webserver with a WS2811 LED Matrix. 

I am using several projects and made adjustments for my own scenario:
(https://github.com/marcmerlin/AnimatedGIFs)
(https://github.com/piotrkochan/esp32-fastled-webserver-platformio#this-is-a-platformio-fork-of-esp32-fastled-web-server)




## Below is _almost_ original README with some of my changes.

Control addressable LEDs with an ESP32 via a web browser over Wi-Fi.

## General

I'm using VS Code and PlatformIO as Toolchain. The Project ist set up with  Espressif ESP32 Dev Module
currently displaying Bitmaps like bitmapsIterationTest(); in main.cpp works fine
However Gifs are displayed wrong, colors are not correctly displayed and it seems like its displaying 2 frames at a time.
Gifs were made with photoshop, then optimized with (https://ezgif.com/optimize).

### Upload data/ to SPIFFS:

```
Gifs are located in Data which are uploaded to SPIFFS

```



## Features
### :

### List:
* [x] Displaying hex arraysbitmaps 
* [x] storing them as json  
* [ ] parse them back into displayable format
* [ ] Displaying Gifs on NeoMatrix Backend
* [ ] Upload gifs via httpPOSt save them into SPIFFS


