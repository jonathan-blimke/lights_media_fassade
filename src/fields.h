/*
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

String getPower() {
  return String(power);
}

String setPower(String value) {
  power = value.toInt();
  power = power == 0 ? 0 : 1;
  return String(power);
}

String getBrightness() {
  return String(brightness);
}

String setBrightness(String value) {
  brightness = value.toInt();
  FastLED.setBrightness(brightness);
  return String(brightness);
}

String getText() {
  return String(serverstring);
}

String setText(String value) {
  serverstring = value;
  return serverstring;
}

String setBitmapArray(String value) {
 array = value;
 return (String) array; 
}

String getBitmapArray() {
  array = "[";
    for( unsigned int a = 0; a < NUM_LEDS; a = a + 1 ) {
      array += bitmap[a];
      if(a < (NUM_LEDS - 1) ){
        array += ",";
      }
      
    }
  array += "]";
  return array;
}

FieldList fields = {
  { "power", "Power", BooleanFieldType, 0, 1, NULL, getPower, NULL, setPower },
  { "brightness", "Brightness", NumberFieldType, 1, 255, NULL, getBrightness, NULL, setBrightness },
  { "text", "Text", TextFieldType, 0, 1, NULL, getText, NULL, setText },
  { "array", "Matrixdata", ArrayFieldType, 0, 1, NULL, getBitmapArray, NULL, setBitmapArray}
};

uint8_t fieldCount = ARRAY_SIZE(fields);

/*
 step one, save bitmap into Fieldlist
        [x] define  name; label; type; ; ; getBitmapArray; ; setBitmapArray;
        [ ] display it on matrix via main
        [ ] save several bitmaps (as frames into it)
        [ ] iterate over bitmaps 
       
  step two, add frontend support 
        [ ] display it via App.js and Bootstrap
        [ ] upload it on Server via postValue, Access body data


*/