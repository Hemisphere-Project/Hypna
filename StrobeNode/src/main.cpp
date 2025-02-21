#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "librmt/esp32_digital_led_lib.h"

const int PIN_LED_RGBW = 26; // 26, 32
const int PIN_LED_RGB = 32; 

int STROBE_FREQ = 100; // dHz
int STROBE_PERIOD = 10000 / STROBE_FREQ; // ms
int STROBE_DUTY = 10; // %

int MASTER1 = 20;
int MASTER2 = 50;
int RED = 255;
int GREEN = 255;
int BLUE = 255;
int WHITE = 255;

int redValue1 = RED * MASTER1 / 255;
int greenValue1 = GREEN * MASTER1 / 255;
int blueValue1 = BLUE * MASTER1 / 255;
int whiteValue1 = WHITE * MASTER1 / 255;

int redValue2 = RED * MASTER2 / 255;
int greenValue2 = GREEN * MASTER2 / 255;
int blueValue2 = BLUE * MASTER2 / 255;
int whiteValue2 = WHITE * MASTER2 / 255;

int delayON = STROBE_PERIOD * STROBE_DUTY / 100;
int delayOFF = STROBE_PERIOD * (100 - STROBE_DUTY) / 100;

const char* ssid = "HYPNA-STROBE";
const char* password = "HypnaStrobe";

strand_t* stripRGBW;
strand_t* stripRGB;

AsyncWebServer server(80);

/*
  WEB CONTENT
*/

const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html>
  <head>
    <title>ESP Web Server</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style>
      html {font-family: Arial; display: inline-block; text-align: left; background-color: black; color: white;}
      h2 {font-size: 2.0rem;}
      p {font-size: 2.0rem;}
      body {max-width: 600px; margin:0px auto; padding-bottom: 25px; font-size: 1.5rem;}
      input[type="text"], input[type="number"]  { font-size:1.5rem; }
    </style>
  </head>
  <body>
    <h2>HYPNA Strobe</h2>
    %PLACEHOLDER%
  <script> 
    function update(e) {
      var value = e.target.value;
      var xhr = new XMLHttpRequest();

      var m1 = document.getElementById("master1").value;
      var m2 = document.getElementById("master2").value;
      var r = document.getElementById("red").value;
      var g = document.getElementById("green").value;
      var b = document.getElementById("blue").value;
      var w = document.getElementById("white").value;
      var f = document.getElementById("freq").value;
      var d = document.getElementById("duty").value;

      xhr.open("GET", "/update?m1=" + m1 + "&m2=" + m2 + "&r=" + r + "&g=" + g + "&b=" + b + "&w=" + w + "&f=" + f + "&d=" + d, true);
      xhr.send();
    }

    document.getElementById("master1").addEventListener("change", update);
    document.getElementById("master2").addEventListener("change", update);

    document.getElementById("red").addEventListener("change", update);
    document.getElementById("green").addEventListener("change", update);
    document.getElementById("blue").addEventListener("change", update);
    document.getElementById("white").addEventListener("change", update);

    document.getElementById("freq").addEventListener("change", update);
    document.getElementById("duty").addEventListener("change", update);

  </script>
  </body>
  </html>
  )rawliteral";

String freq(){
  return String(STROBE_FREQ/10.0);
}

String duty(){
  return String(STROBE_DUTY);
}

String master1(){
  return String(MASTER1);
}

String master2(){
  return String(MASTER2);
}

String red(){
  return String(RED);
}

String green(){
  return String(GREEN);
}

String blue(){
  return String(BLUE);
}

String white(){
  return String(WHITE);
}

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "PLACEHOLDER"){
    String buttons = "";
    buttons += "<table><tr>";
    buttons += "<td><input type=\"number\" step=\"0.1\" min=\"0.1\" max=\"100\" id=\"freq\" value=\""+freq()+"\" ></td>";
    buttons += "<td><label for=\"freq\">Frequency</label></td>";
    buttons += "<td>&nbsp;&nbsp;</td>";
    buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"red\" value=\""+red()+"\" ></td>";
    buttons += "<td><label for=\"red\">Red</label></td>";
    buttons += "</tr><tr>";
    buttons += "<td><input type=\"number\" step=\"1\" min=\"0\" max=\"100\"  id=\"duty\"  value=\""+duty()+"\" ></td>";
    buttons += "<td><label for=\"duty\">Duty %</label></td>";
    buttons += "<td></td>";
    buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"green\" value=\""+green()+"\" ></td>";
    buttons += "<td><label for=\"green\">Green</label></td>";
    buttons += "</tr><tr>";
    buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"master1\" value=\""+master1()+"\" ></td>";
    buttons += "<td><label for=\"master\">Master RGBW</label></td>";
    buttons += "<td></td>";
    buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"blue\" value=\""+blue()+"\" ></td>";
    buttons += "<td><label for=\"blue\">Blue</label></td>";
    buttons += "</tr><tr>";
    buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"master2\" value=\""+master2()+"\" ></td>";
    buttons += "<td><label for=\"master\">Master RGB</label></td>";
    buttons += "<td></td>";
    buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"white\" value=\""+white()+"\" ></td>";
    buttons += "<td><label for=\"white\">White</label></td>";
    buttons += "</tr></table>";
    
    return buttons;
  }
  return String();
}


/* 
  ARDUINO SCRIPT
*/

void setup() 
{
  Serial.begin(115200);
  digitalLeds_init();
  
  stripRGBW = digitalLeds_addStrand(
          {.rmtChannel = 0, .gpioNum = PIN_LED_RGBW, .ledType = LED_SK6812W_V3, .brightLimit = 255, .numPixels = 1, .pixels = nullptr, ._stateVars = nullptr});
  stripRGB = digitalLeds_addStrand(
          {.rmtChannel = 1, .gpioNum = PIN_LED_RGB, .ledType = LED_WS2811_HS, .brightLimit = 255, .numPixels = 1, .pixels = nullptr, ._stateVars = nullptr});
  
  // wifi connect
  // WiFi.begin(ssid, password);

  WiFi.softAPConfig (IPAddress (10, 0, 0, 1), IPAddress (10, 0, 0, 1), IPAddress (255, 255, 255, 0));
  WiFi.softAP(ssid, password);

  Serial.print("Connecting to WiFi...");
  // while (WiFi.status() != WL_CONNECTED) 
  // {
  //   delay(500);
  //   Serial.print(".");
  // }
  Serial.println("OK!");
  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());


  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    if (request->hasParam("m1")) {
      inputMessage = request->getParam("m1")->value();
      MASTER1 = inputMessage.toInt();
      if (MASTER1 < 0) MASTER1 = 0;
      if (MASTER1 > 255) MASTER1 = 255;
      Serial.printf("Master1: %d\n", MASTER1);
    }
    if (request->hasParam("m2")) {
      inputMessage = request->getParam("m2")->value();
      MASTER2 = inputMessage.toInt();
      if (MASTER2 < 0) MASTER2 = 0;
      if (MASTER2 > 255) MASTER2 = 255;
      Serial.printf("Master2: %d\n", MASTER2);
    }
    if (request->hasParam("r")) {
      inputMessage = request->getParam("r")->value();
      RED = inputMessage.toInt();
      if (RED < 0) RED = 0;
      if (RED > 255) RED = 255;
      Serial.printf("Red: %d\n", RED);
    }
    if (request->hasParam("g")) {
      inputMessage = request->getParam("g")->value();
      GREEN = inputMessage.toInt();
      if (GREEN < 0) GREEN = 0;
      if (GREEN > 255) GREEN = 255;
      Serial.printf("Green: %d\n", GREEN);
    }
    if (request->hasParam("b")) {
      inputMessage = request->getParam("b")->value();
      BLUE = inputMessage.toInt();
      if (BLUE < 0) BLUE = 0;
      if (BLUE > 255) BLUE = 255;
      Serial.printf("Blue: %d\n", BLUE);
    }
    if (request->hasParam("w")) {
      inputMessage = request->getParam("w")->value();
      WHITE = inputMessage.toInt();
      if (WHITE < 0) WHITE = 0;
      if (WHITE > 255) WHITE = 255;
      Serial.printf("White: %d\n", WHITE);
    }
    if (request->hasParam("d")) {
      inputMessage = request->getParam("d")->value();
      STROBE_DUTY = inputMessage.toInt();
      if (STROBE_DUTY < 0) STROBE_DUTY = 0;
      if (STROBE_DUTY > 100) STROBE_DUTY = 100;
      Serial.printf("Duty: %d\n", STROBE_DUTY);
    }
    if (request->hasParam("f")) {
      inputMessage = request->getParam("f")->value();
      STROBE_FREQ = inputMessage.toFloat() * 10;
      STROBE_PERIOD = 10000 / STROBE_FREQ;
      if (STROBE_PERIOD < 0) STROBE_PERIOD = 0;
      Serial.printf("Freq: %d\n", STROBE_FREQ);
    }

    // calculate new values
    redValue1 = RED * MASTER1 / 255;
    greenValue1 = GREEN * MASTER1 / 255;
    blueValue1 = BLUE * MASTER1 / 255;
    whiteValue1 = WHITE * MASTER1 / 255;

    redValue2 = RED * MASTER2 / 255;
    greenValue2 = GREEN * MASTER2 / 255;
    blueValue2 = BLUE * MASTER2 / 255;
    whiteValue2 = WHITE * MASTER2 / 255;

    delayON = STROBE_PERIOD * STROBE_DUTY / 100;
    delayOFF = STROBE_PERIOD * (100 - STROBE_DUTY) / 100;

    request->send(200, "text/plain", "OK");
  });

    
  // Start server
  server.begin();
}

void loop() 
{
  /*
    ON
  */

  stripRGBW->pixels[0] = pixelFromRGBW(greenValue1, redValue1, blueValue1, whiteValue1);
  for (int i = 1; i < stripRGBW->numPixels; i++) stripRGBW->pixels[i] = stripRGBW->pixels[0];
  digitalLeds_updatePixels(stripRGBW);

  stripRGB->pixels[0] = pixelFromRGB(redValue2, greenValue2, blueValue2);
  for (int i = 1; i < stripRGB->numPixels; i++) stripRGB->pixels[i] = stripRGB->pixels[0];
  digitalLeds_updatePixels(stripRGB);

  delay(delayON);

  /*
    OFF
  */

  stripRGBW->pixels[0] = pixelFromRGBW(0, 0, 0, 0);
  for (int i = 1; i < stripRGBW->numPixels; i++) stripRGBW->pixels[i] = stripRGBW->pixels[0];
  digitalLeds_updatePixels(stripRGBW);
  
  stripRGB->pixels[0] = pixelFromRGB(0, 0, 0);
  for (int i = 1; i < stripRGB->numPixels; i++) stripRGB->pixels[i] = stripRGB->pixels[0];
  digitalLeds_updatePixels(stripRGB);
  
  delay(delayOFF);
}