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
 /*
    input string = "25353, 12412, ..."
    main array data = input

 */
 return (String) array; 
}

String getBitmapArray() {
  // array = "[";
  //   for( unsigned int a = 1; a < NUMMATRIX; a = a + 1 ) {
  //     array += bitmap[a];
  //     if(a < (NUM_LEDS - 1) ){
  //       array += ",";
  //     }
      
  //   }
  // array += "]";
  return (String)array;
}

String arrayToString(uint16_t *array){
  String returnvalue ="init";

  for( int a = 1; a < NUMMATRIX; a = a + 1 ) {
      returnvalue += array[a];
      if(a < (NUM_LEDS - 1) ){
        returnvalue += ",";
      } 
    }
    return returnvalue;
}

uint16_t* stringToArray(String str) {
    int str_length = NUM_LEDS; 
    uint16_t arr[str_length] = { 0 }; 
    int j = 0, i;
  
    for (i = 1; i <  NUMMATRIX ; i++) {  //NUMMATRIX
        // if str[i] is ', ' then split 
        if (str[i] == ',') { 
            j++; // Increment j to point to next array index
        } 
        else { 
            // subtract str[i] by 48 to convert it to int 
            // generate dezimal number by *10  
            arr[j] = arr[j] * 10 + (str[i] - 48);
        } 
    }

  return arr;

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
        [x] iterate over bitmaps 
       
  step two, add frontend support 
        [ ] display it via App.js and Bootstrap
        [ ] upload it on Server via postValue, Access body data


*/