/*
 * SPDX-FileCopyrightText: 2019 Ha Thach (tinyusb.org)
 *
 * SPDX-License-Identifier: MIT
 *
 * SPDX-FileContributor: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-FileContributor: 2023 esp32beans@gmail.com
 */

// This program is based on an ESP-IDF USB MIDI TinyUSB example with minimal
// changes so it works on Arduino-esp32.

#include <Arduino.h>
#include "usbmidi.h"


void setup() {
  Serial.begin(115200);

  usbmidi_init();
}

void loop() {}

// #include <Arduino.h>
// // #include <WiFi.h>
// // #include <AsyncTCP.h>
// // #include <ESPAsyncWebServer.h>


// #include "nowlink.h"
// #include "tusb.h"


// int STROBE_FREQ = 50; // dHz
// int STROBE_PERIOD = 10000 / STROBE_FREQ; // ms
// int STROBE_DUTY = 10; // %

// int MASTER1 = 100;
// int RED = 255;
// int GREEN = 255;
// int BLUE = 255;
// int WHITE = 255;

// const char* ssid = "HYPNA-STROBE";
// const char* password = "HypnaStrobe";


// // AsyncWebServer server(80);

// /*
//   WEB CONTENT
// */

// const char index_html[] PROGMEM = R"rawliteral(
//   <!DOCTYPE HTML><html>
//   <head>
//     <title>ESP Web Server</title>
//     <meta name="viewport" content="width=device-width, initial-scale=1">
//     <link rel="icon" href="data:,">
//     <style>
//       html {font-family: Arial; display: inline-block; text-align: left; background-color: black; color: white;}
//       h2 {font-size: 2.0rem;}
//       p {font-size: 2.0rem;}
//       body {max-width: 600px; margin:0px auto; padding-bottom: 25px; font-size: 1.5rem;}
//       input[type="text"], input[type="number"]  { font-size:1.5rem; }
//     </style>
//   </head>
//   <body>
//     <h2>HYPNA Strobe</h2>
//     %PLACEHOLDER%
//   <script> 
//     function update(e) {
//       var value = e.target.value;
//       var xhr = new XMLHttpRequest();

//       var m1 = document.getElementById("master1").value;
//       var m2 = document.getElementById("master2").value;
//       var r = document.getElementById("red").value;
//       var g = document.getElementById("green").value;
//       var b = document.getElementById("blue").value;
//       var w = document.getElementById("white").value;
//       var f = document.getElementById("freq").value;
//       var d = document.getElementById("duty").value;

//       xhr.open("GET", "/update?m1=" + m1 + "&m2=" + m2 + "&r=" + r + "&g=" + g + "&b=" + b + "&w=" + w + "&f=" + f + "&d=" + d, true);
//       xhr.send();
//     }

//     document.getElementById("master1").addEventListener("change", update);
//     document.getElementById("master2").addEventListener("change", update);

//     document.getElementById("red").addEventListener("change", update);
//     document.getElementById("green").addEventListener("change", update);
//     document.getElementById("blue").addEventListener("change", update);
//     document.getElementById("white").addEventListener("change", update);

//     document.getElementById("freq").addEventListener("change", update);
//     document.getElementById("duty").addEventListener("change", update);

//   </script>
//   </body>
//   </html>
//   )rawliteral";

// String freq(){
//   return String(STROBE_FREQ/10.0);
// }

// String duty(){
//   return String(STROBE_DUTY);
// }

// String master1(){
//   return String(MASTER1);
// }

// String red(){
//   return String(RED);
// }

// String green(){
//   return String(GREEN);
// }

// String blue(){
//   return String(BLUE);
// }

// String white(){
//   return String(WHITE);
// }

// // Replaces placeholder with button section in your web page
// String processor(const String& var){
//   //Serial.println(var);
//   if(var == "PLACEHOLDER"){
//     String buttons = "";
//     buttons += "<table><tr>";
//     buttons += "<td><input type=\"number\" step=\"0.1\" min=\"0.1\" max=\"100\" id=\"freq\" value=\""+freq()+"\" ></td>";
//     buttons += "<td><label for=\"freq\">Frequency</label></td>";
//     buttons += "<td>&nbsp;&nbsp;</td>";
//     buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"red\" value=\""+red()+"\" ></td>";
//     buttons += "<td><label for=\"red\">Red</label></td>";
//     buttons += "</tr><tr>";
//     buttons += "<td><input type=\"number\" step=\"1\" min=\"0\" max=\"100\"  id=\"duty\"  value=\""+duty()+"\" ></td>";
//     buttons += "<td><label for=\"duty\">Duty %</label></td>";
//     buttons += "<td></td>";
//     buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"green\" value=\""+green()+"\" ></td>";
//     buttons += "<td><label for=\"green\">Green</label></td>";
//     buttons += "</tr><tr>";
//     buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"master1\" value=\""+master1()+"\" ></td>";
//     buttons += "<td><label for=\"master\">Master RGBW</label></td>";
//     buttons += "<td></td>";
//     buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"blue\" value=\""+blue()+"\" ></td>";
//     buttons += "<td><label for=\"blue\">Blue</label></td>";
//     buttons += "</tr><tr>";
//     buttons += "<td></td>";
//     buttons += "<td></td>";
//     buttons += "<td></td>";
//     buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"white\" value=\""+white()+"\" ></td>";
//     buttons += "<td><label for=\"white\">White</label></td>";
//     buttons += "</tr></table>";
    
//     return buttons;
//   }
//   return String();
// }

// void handleNoteOn(byte channel, byte note, byte velocity) {
//   Serial.printf("NoteOn  Ch:%d Note:%d Vel:%d\n", channel, note, velocity);
// }

// void handleNoteOff(byte channel, byte note, byte velocity) {
//   Serial.printf("NoteOff Ch:%d Note:%d Vel:%d\n", channel, note, velocity);
// }

// void handleCC(uint8_t cc, uint8_t value) {
//   switch(cc) {
//     case 100: MASTER1 = value*2; break;
//     case 101: RED = value*2; break;
//     case 102: GREEN = value*2; break;
//     case 103: BLUE = value*2; break;
//     case 104: WHITE = value*2; break;
//     case 105: 
//       STROBE_FREQ = 200.0 + (value * 10.0 / 4.0); 
//       STROBE_PERIOD = 10000 / STROBE_FREQ;
//       break;
//     case 106: STROBE_DUTY = value * 100.0 / 127.0; break;
//   }
  
//   Serial.printf("CC %d: %.1f\n", cc, value * 100.0 / 127.0);
// }

// // MIDI packet structure
// struct midi_packet {
//   uint8_t header;
//   uint8_t byte1;
//   uint8_t byte2;
//   uint8_t byte3;
// };

// // USB Descriptors (Critical for Device Recognition)
// uint8_t const desc_midi[] = {
//   // Interface count, string indexes, etc
//   0x09, 0x04, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00,
//   0x09, 0x24, 0x01, 0x00, 0x01, 0x09, 0x00, 0x01, 0x01,
//   0x09, 0x04, 0x01, 0x00, 0x02, 0x01, 0x03, 0x00, 0x00,
//   0x07, 0x24, 0x01, 0x00, 0x01, 0x41, 0x00,
// };

// // USB MIDI Descriptor (Critical for Device Recognition)
// tusb_desc_device_t desc_dev = {
//   .bLength            = sizeof(tusb_desc_device_t),
//   .bDescriptorType    = TUSB_DESC_DEVICE,
//   .bcdUSB             = 0x0200,
//   .bDeviceClass       = TUSB_CLASS_MISC,
//   .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
//   .bDeviceProtocol    = MISC_PROTOCOL_IAD,
//   .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
//   .idVendor           = 0x1234,
//   .idProduct          = 0x5678,
//   .bcdDevice          = 0x0100,
//   .iManufacturer      = 0x01,
//   .iProduct           = 0x02,
//   .iSerialNumber      = 0x03,
//   .bNumConfigurations = 0x01
// };

// // USB Initialization
// extern "C" {
//   uint8_t const* tud_descriptor_device_cb(void) { return (uint8_t const*)&desc_dev; }
//   uint8_t const* tud_descriptor_configuration_cb(uint8_t index) { return desc_midi; }
//   }


// /* 
//   ARDUINO SCRIPT
// */

// void setup() 
// {
//   Serial.begin(115200);

//   nowInit(NULL);
//   Serial.println("Nowlink initialized");

//   // Manual USB Initialization
//   USB.productName("ESP32-S3 MIDI");
//   USB.manufacturerName("Your Company");
//   USB.serialNumber("123456");
//   USB.begin();



//   // WiFi.softAPConfig (IPAddress (10, 0, 0, 1), IPAddress (10, 0, 0, 1), IPAddress (255, 255, 255, 0));
//   // WiFi.softAP(ssid, password);
//   // Serial.print("Connecting to WiFi...");
//   // Serial.println("OK!");

//   // // Route for root / web page
//   // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//   //   request->send_P(200, "text/html", index_html, processor);
//   // });

//   // server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
//   //   String inputMessage;
//   //   if (request->hasParam("m1")) {
//   //     inputMessage = request->getParam("m1")->value();
//   //     MASTER1 = inputMessage.toInt();
//   //     if (MASTER1 < 0) MASTER1 = 0;
//   //     if (MASTER1 > 255) MASTER1 = 255;
//   //     Serial.printf("Master1: %d\n", MASTER1);
//   //   }
//   //   if (request->hasParam("r")) {
//   //     inputMessage = request->getParam("r")->value();
//   //     RED = inputMessage.toInt();
//   //     if (RED < 0) RED = 0;
//   //     if (RED > 255) RED = 255;
//   //     Serial.printf("Red: %d\n", RED);
//   //   }
//   //   if (request->hasParam("g")) {
//   //     inputMessage = request->getParam("g")->value();
//   //     GREEN = inputMessage.toInt();
//   //     if (GREEN < 0) GREEN = 0;
//   //     if (GREEN > 255) GREEN = 255;
//   //     Serial.printf("Green: %d\n", GREEN);
//   //   }
//   //   if (request->hasParam("b")) {
//   //     inputMessage = request->getParam("b")->value();
//   //     BLUE = inputMessage.toInt();
//   //     if (BLUE < 0) BLUE = 0;
//   //     if (BLUE > 255) BLUE = 255;
//   //     Serial.printf("Blue: %d\n", BLUE);
//   //   }
//   //   if (request->hasParam("w")) {
//   //     inputMessage = request->getParam("w")->value();
//   //     WHITE = inputMessage.toInt();
//   //     if (WHITE < 0) WHITE = 0;
//   //     if (WHITE > 255) WHITE = 255;
//   //     Serial.printf("White: %d\n", WHITE);
//   //   }
//   //   if (request->hasParam("d")) {
//   //     inputMessage = request->getParam("d")->value();
//   //     STROBE_DUTY = inputMessage.toInt();
//   //     if (STROBE_DUTY < 0) STROBE_DUTY = 0;
//   //     if (STROBE_DUTY > 100) STROBE_DUTY = 100;
//   //     Serial.printf("Duty: %d\n", STROBE_DUTY);
//   //   }
//   //   if (request->hasParam("f")) {
//   //     inputMessage = request->getParam("f")->value();
//   //     STROBE_FREQ = inputMessage.toFloat() * 10;
//   //     STROBE_PERIOD = 10000 / STROBE_FREQ;
//   //     if (STROBE_PERIOD < 0) STROBE_PERIOD = 0;
//   //     Serial.printf("Freq: %d\n", STROBE_FREQ);
//   //   }

//   //   // calculate new values
//   //   redValue1 = RED * MASTER1 / 255;
//   //   greenValue1 = GREEN * MASTER1 / 255;
//   //   blueValue1 = BLUE * MASTER1 / 255;
//   //   whiteValue1 = WHITE * MASTER1 / 255;

//   //   delayON = STROBE_PERIOD * STROBE_DUTY / 100;
//   //   delayOFF = STROBE_PERIOD * (100 - STROBE_DUTY) / 100;

//   //   request->send(200, "text/plain", "OK");
//   // });

    
//   // // Start server
//   // server.begin();
// }

// uint64_t lastTime = 0;

// void loop() 
// {
//   midi_packet packet;
  
//   // Read MIDI packets from USB
//   while(tud_midi_available()) {
//     tud_midi_packet_read((uint8_t*)&packet);
    
//     // Check for Control Change message (0xB)
//     if((packet.header & 0x0F) == 0x0B) {
//       uint8_t cc = packet.byte2;
//       uint8_t value = packet.byte3;
//       handleCC(cc, value);
//     }
//   }

//   delay(1);
// }
