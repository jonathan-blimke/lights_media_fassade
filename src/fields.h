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
// void printTHATArray(std::vector<uint16_t> input) {
//   std::vector<uint16_t>::iterator iter;  
//   int i = 0;  //data counter

//   for(iter = input.begin(); iter != input.end(); ++iter,i++ ) {
//     Serial.print(*iter);
//     Serial.print(", ");
//   }
// }

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

//converts an array of integers into an JSON conform string
String arrayToString(std::vector<uint16_t> frames) {
  String returnvalue; //return String in JSON array syntax
  returnvalue +="[";
  std::vector<uint16_t>::iterator iter;  
  int i = 0;  //counter

  for(iter = frames.begin(); iter != frames.end(); iter++,i++ ) {
    returnvalue += *iter;
    if( i < frames.size()-1) {
      returnvalue += ",";
    }
    // if(iter != frames.end()) { 
    //  returnvalue += ",";
    // }

  }
    returnvalue +="]";
    return returnvalue;
}


void clearNBitmap(){
  for(int i = 0; i < NUMMATRIX*2; i ++){
    bitmapNframes[i] = 0;
  }
}

void clearValueBuffer() {
 for(int i = 0; i < 10000; i++){
   valueBuffer[i] = 0;
 }
 
  
}

// template <typename Out>
// void split(const std::string &s, char delim, Out result) {
//     std::istringstream iss(s);
//     std::string item;
//     while (std::getline(iss, item, delim)) {
//         *result++ = item;
//     }
// }
// //usage:  std::vector<std::string> stringValues = split(str.c_str(),',');
// std::vector<std::string> split(const std::string &s, char delim) {
//     std::vector<std::string> elems;
//     split(s, delim, std::back_inserter(elems));
//     return elems;

// }

void clearFrameData() {
 std::vector<uint16_t>::iterator iter;  
  int i = 0;  //data counter

  for(iter = frameData.begin(); iter != frameData.end(); ++iter,i++ ) {
    frameData.erase(iter); //last change
  }
}
//1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60

//parse's string into std::vector<uint16:t> frameData
std::vector<uint16_t> stringToArray(String str) {   
    int j = 0, i;
  //  clearFrameData(); //uncomment
   frameData.clear(); //last change
   clearValueBuffer();
   std::stringstream sstr(str.c_str());
   
   std::string token;
   size_t size =0;
   while(std::getline(sstr, token, ',')) {
    //  Serial.print(token.c_str());
     std::stringstream ss(token.c_str());
     uint16_t intvalue;
     ss >> intvalue;
     Serial.print(intvalue, HEX);
     frameData.push_back(intvalue);
    //  uint16_t int = std::stoi(token.c_str());
      // size_t length = token.length();
      // size=size + 2;
    //  std::istringstream strm(token.substr(size, 2));
    //  uint16_t x;
    //  strm >> std::dec >> x;
    //  Serial.print(x, DEC);
    // frameData.push_back(x);
   }
  
    
    // for (i = 0; i <  str.length() ; i++) { 
    //     // if str[i] is ', ' then split 
    //     if (str[i] == ',') { 
    //         j++; // calculates numbers of Values
        
    //     }
    //     else {
    //       valueBuffer[j] = valueBuffer[j] * 10 + (str[i]- 48);
    //       //replace with vector
    //       //write at specific position v.at(1)
    //     }
    // }

    // for(int a=0; a <= j;a++) { //valueBuffers pushes wrong data
    //   frameData.push_back(valueBuffer[a]);
    // }
    
    // printTHATArray(frameData);
    return frameData;  
} 

//if string contains more than NUMMMATRIX values
uint16_t* stringFramesToArray(String str) {   
    int j = 0, i;
    clearNBitmap();
    
    for (i = 0; i <  str.length() ; i++) {  //NUMMATRIX
        // if str[i] is ', ' then split 
        if (str[i] == ',') { 
            j++; // Increment j to point to next array index
        }  else {
          bitmapNframes[j] = bitmapNframes[j] * 10 + (str[i] - 48);
        }
      
    }
    
    // Serial.print("stringToArray");
    // printTHATArray(bitmapNframes);
    return bitmapNframes;
  
} 


String getBitmapArray() {
  return arrayToString(frameData);
  
}

String setBitmapArray(String value) {
 stringToArray(value);
 return arrayToString(frameData); 
}



FieldList fields = {
  { "power", "Power", BooleanFieldType, 0, 1, NULL, getPower, NULL, setPower },
  { "brightness", "Brightness", NumberFieldType, 1, 255, NULL, getBrightness, NULL, setBrightness },
  { "text", "Text", TextFieldType, 0, 1, NULL, getText, NULL, setText },
  { "array", "Matrixdata", ArrayFieldType, 0, 1, NULL, getBitmapArray, NULL, setBitmapArray}
};

uint8_t fieldCount = ARRAY_SIZE(fields);
