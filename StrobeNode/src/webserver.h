#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


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
      input[type="text"], input[type="number"]  { font-size:1.5rem; width: 80px; }
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
      var r = document.getElementById("red").value;
      var g = document.getElementById("green").value;
      var b = document.getElementById("blue").value;
      var w = document.getElementById("white").value;
      var f = document.getElementById("freq").value;
      var d = document.getElementById("duty").value;

      xhr.open("GET", "/update?m1=" + m1 + "&r=" + r + "&g=" + g + "&b=" + b + "&w=" + w + "&f=" + f + "&d=" + d, true);
      xhr.send();
    }

    document.getElementById("master1").addEventListener("change", update);

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


// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "PLACEHOLDER"){
    String buttons = "";
    buttons += "<table><tr>";
    buttons += "<td><input type=\"number\" step=\"0.1\" min=\"0.1\" max=\"100\" id=\"freq\" value=\""+String(strobe_getFreq())+"\" ></td>";
    buttons += "<td><label for=\"freq\">Frequency</label></td>";
    buttons += "<td>&nbsp;&nbsp;</td>";
    buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"red\" value=\""+String(strobe_getRed())+"\" ></td>";
    buttons += "<td><label for=\"red\">Red</label></td>";
    buttons += "</tr><tr>";
    buttons += "<td><input type=\"number\" step=\"1\" min=\"0\" max=\"100\"  id=\"duty\"  value=\""+String(strobe_getDuty())+"\" ></td>";
    buttons += "<td><label for=\"duty\">Duty %</label></td>";
    buttons += "<td></td>";
    buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"green\" value=\""+String(strobe_getGreen())+"\" ></td>";
    buttons += "<td><label for=\"green\">Green</label></td>";
    buttons += "</tr><tr>";
    buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"master1\" value=\""+String(strobe_getMaster1())+"\" ></td>";
    buttons += "<td><label for=\"master\">Master RGBW</label></td>";
    buttons += "<td></td>";
    buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"blue\" value=\""+String(strobe_getBlue())+"\" ></td>";
    buttons += "<td><label for=\"blue\">Blue</label></td>";
    buttons += "</tr><tr>";
    buttons += "<td></td>";
    buttons += "<td></td>";
    buttons += "<td></td>";
    buttons += "<td><input type=\"number\" min=\"0\" max=\"255\" id=\"white\" value=\""+String(strobe_getWhite())+"\" ></td>";
    buttons += "<td><label for=\"white\">White</label></td>";
    buttons += "</tr></table>";
    
    return buttons;
  }
  return String();
}

void webserver_init()
{
    WiFi.softAPConfig (IPAddress (10, 0, 0, 1), IPAddress (10, 0, 0, 1), IPAddress (255, 255, 255, 0));
    WiFi.softAP(ssid, password);
    Serial.print("Connecting to WiFi...");
    Serial.println("OK!");

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
    });

    server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {

        int MASTER1 = strobe_getMaster1();
        int RED = strobe_getRed();
        int GREEN = strobe_getGreen();
        int BLUE = strobe_getBlue();
        int WHITE = strobe_getWhite();
        int STROBE_DUTY = strobe_getDuty();
        float STROBE_FREQ = strobe_getFreq();

        String inputMessage;
        if (request->hasParam("m1")) {
            inputMessage = request->getParam("m1")->value();
            MASTER1 = inputMessage.toInt();
            if (MASTER1 < 0) MASTER1 = 0;
            if (MASTER1 > 255) MASTER1 = 255;
            Serial.printf("Master1: %d\n", MASTER1);
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
            STROBE_FREQ = inputMessage.toFloat();
            Serial.printf("Freq: %d\n", STROBE_FREQ);
        }

        strobe_set(STROBE_FREQ, STROBE_DUTY, MASTER1, RED, GREEN, BLUE, WHITE);

        // Send response
        request->send(200, "text/plain", "OK");
    });


    // Start server
    server.begin();
}