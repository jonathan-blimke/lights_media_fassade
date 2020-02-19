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
void printTHATArray(uint16_t* array) {
  
  for (int b = 0; b < NUMMATRIX; b++) {
    Serial.print(array[b]);
    Serial.print(","); 
  }
}

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
  String result ="\"";
  result += value;
  result += "\"";
  serverstring = result;
  return serverstring;
}

String decToHex(int dec_value) {
  std::stringstream strs;
  strs << std::hex << dec_value;
  std::string hexstr(strs.str());
  // Serial.println(hexstr.c_str());
  
  return hexstr.c_str();
}

//converts an array of integers into an String with a "," as separator
String arrayToString(uint16_t *array) {
  String returnvalue;
  returnvalue +="[";
  for( int a = 0; a < NUMMATRIX; a = a + 1 ) {
       
      // returnvalue += decToHex(array[a]);  //to write hex values in json
      
      returnvalue += array[a]; //to write decimal values into json
      if(a < (NUMMATRIX - 1) ) { //changed from NUM_LEDS
        returnvalue += ",";
      } 
    }
    returnvalue +="]";
    return returnvalue;
}

void clearBitmap(){
  for(int i = 0; i < NUMMATRIX; i ++){
    bitmap[i] = 0;
  }
}

//converts decimal String in int signatur like "42" into an actual decimal int = 42
uint16_t* stringToArray(String str) {   
    int j = 0, i;
    clearBitmap();
    for (i = 0; i <  str.length() ; i++) {  //NUMMATRIX
        // if str[i] is ', ' then split 
        if (str[i] == ',') { 
            j++; // Increment j to point to next array index
        }
        else { 
    
            // Serial.print(" j=" );
            // Serial.print(j);
            // Serial.print(", bitmap= ");
            // Serial.print(bitmap[j]);
            bitmap[j] = bitmap[j] * 10 + (str[i] - 48);
        }
    }
    // Serial.print("stringToArray");
    // printTHATArray(bitmap);
    return bitmap;
  
} 


String getBitmapArray() {
  Serial.print("getBitmapArray= ");
  Serial.print(arrayToString(bitmap));
  return arrayToString(bitmap);
  
}

String setBitmapArray(String value) {
  Serial.print("sssetBitmapArray= ");
  Serial.print(value);
  stringToArray(value);
 
 return arrayToString(bitmap); 
}



FieldList fields = {
  { "power", "Power", BooleanFieldType, 0, 1, NULL, getPower, NULL, setPower },
  { "brightness", "Brightness", NumberFieldType, 1, 255, NULL, getBrightness, NULL, setBrightness },
  { "text", "Text", TextFieldType, 0, 1, NULL, getBitmapArray, NULL, setBitmapArray },
  { "array", "Matrixdata", ArrayFieldType, 0, 1, NULL, getBitmapArray, NULL, setBitmapArray}
};

uint8_t fieldCount = ARRAY_SIZE(fields);
