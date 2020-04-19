# Welcome
LED Matrix Interface


## General
This Project was created with VS Code and PlatformIO IDE. You can control an LED Matrix via a Network. Build with FastLED and FastLED_Neomatrix. To ask resources you can send GET requests to the API endpoint. Sending Data, such as content that you want to display on the LED Matrix, can be send via POST Methods.

### GET Method example
GET /all HTTP/1.1   <br />
Host: 192.168.0.2  <br />
Accept: application/json <br />

### POST Method example
POST /fieldValue HTTP/1.1 <br />
Host: 192.168.0.2 <br />
User-Agent: insomnia/7.1.1 <br />
Content-Type: application/x-www-form-urlencoded Accept: */* <br />
Content-Length: 274 <br />

name=rgb888A_HEX&value=b5541f,b5541f,000000,000000,fb00e6,fb00e6, b5541f,000000,000000,000000,000000,fb00e6,000000,000000,000000,000000, 000000,000000,000000,000000,000000,000000,000000,000000,53c64e,000000, 000000,000000,000000,290a59,53c64e,53c64e,000000,000000,290a59,290a59

### adressable resources:
* [x] power
* [x] brightness
* [x] text
* [x] rgb565_DEC
* [x] displaybitmap
* [x] fps
* [x] rgb888A_HEX


### Bitmapconverter
The file "bitmapconverter.html" is a tool to create graphic content for your led matrix. It parses an .bmp into text which is seeable in the POST example. 

