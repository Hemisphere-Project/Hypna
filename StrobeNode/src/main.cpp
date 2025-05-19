#include <Arduino.h>

const int PIN_LED_RGBW = 32; // 26: yellow, 32: white 
const int PIN_TRIGGER  = 26; // 26: yellow, 32: white
const int PIN_LED_INTERNAL = 27;

const char* ssid = "HYPNA-STROBE";
const char* password = "HypnaStrobe";

#include "nowlink.h"
#include "strobe.h"
#include "webserver.h"
#include "librmt/esp32_digital_led_lib.h"


strand_t* stripRGBW;
strand_t* stripInternal;


/* 
  ARDUINO SCRIPT
*/

void setup() 
{
  Serial.begin(115200);
  digitalLeds_init();

  stripRGBW = digitalLeds_addStrand(
          {.rmtChannel = 0, .gpioNum = PIN_LED_RGBW, .ledType = LED_SK6812W_V3, .brightLimit = 255, .numPixels = 8, .pixels = nullptr, ._stateVars = nullptr});
  
  stripInternal = digitalLeds_addStrand(
          {.rmtChannel = 1, .gpioNum = PIN_LED_INTERNAL, .ledType = LED_WS2812B_V3, .brightLimit = 255, .numPixels = 1, .pixels = nullptr, ._stateVars = nullptr});

  
  // trigger pin (for measurement)
  pinMode(PIN_TRIGGER, OUTPUT);
  digitalWrite(PIN_TRIGGER, LOW);
  
  // Strobe init
  strobe_set(1, 10, 255, 255, 255, 255, 255); // 1Hz, 10% duty, Master 255, RGBW 255

  // ESP-NOW timesync
  nowInit(
    // RECEIVE CALLBACK
    //
    [](uint32_t from, String msg) {
      // message: settings S=timestampµs/Freq/Duty/Master1/Red/Green/Blue/White
      if (msg.startsWith("S=")) {
          // parse settings
          uint32_t timestamp = msg.substring(2, msg.indexOf('/')).toInt();
          String settings = msg.substring(msg.indexOf('/') + 1);
          float Freq = settings.substring(0, settings.indexOf('/')).toFloat();
          settings.remove(0, settings.indexOf('/') + 1);
          int Duty = settings.substring(0, settings.indexOf('/')).toInt();
          settings.remove(0, settings.indexOf('/') + 1);
          int Master1 = settings.substring(0, settings.indexOf('/')).toInt();
          settings.remove(0, settings.indexOf('/') + 1);
          int Red = settings.substring(0, settings.indexOf('/')).toInt(); 
          settings.remove(0, settings.indexOf('/') + 1);
          int Green = settings.substring(0, settings.indexOf('/')).toInt();
          settings.remove(0, settings.indexOf('/') + 1);
          int Blue = settings.substring(0, settings.indexOf('/')).toInt();
          settings.remove(0, settings.indexOf('/') + 1);
          int White = settings.toInt();
          
          // set settings
          // Serial.printf("REMOTE Settings: ");
          strobe_set(timestamp, Freq, Duty, Master1, Red, Green, Blue, White);
      }
    }, 
    // INFO CALLBACK
    //
    []() {
      // send settings
      nowBroadcast("S=" + String(strobe_getTS()) + "/" + String(strobe_getFreq()) + "/" + String(strobe_getDuty()) + "/" + String(strobe_getMaster1()) + "/" +
        String(strobe_getRed()) + "/" + String(strobe_getGreen()) + "/" + String(strobe_getBlue()) + "/" + String(strobe_getWhite()));
      // Serial.printf("INFO: %s\n", msg.c_str());
    }
  );

  // Webserver
  webserver_init();
}

int busylooped = 0;
uint64_t unow, lastTime = 0;
void loop() 
{
  // Set ON
  if (strobe_update(nowMicros())) 
  {
    digitalWrite(PIN_TRIGGER, strobe_state() ? HIGH : LOW);
    
    stripRGBW->pixels[0] = strobe_get();
    for (int i = 1; i < stripRGBW->numPixels; i++) stripRGBW->pixels[i] = stripRGBW->pixels[0];
    digitalLeds_updatePixels(stripRGBW);
    
    for (int i = 0; i < stripInternal->numPixels; i++) stripInternal->pixels[i] = stripRGBW->pixels[0];
    digitalLeds_updatePixels(stripInternal);
    
    if (busylooped == 0) Serial.println("WARNING: no busylooped, increase busytime in strobe_sleep");
    if (busylooped > 10) Serial.printf("WARNING: busylooped a lot (x%d), decrease busytime in strobe_sleep\n", busylooped);
    busylooped = 0;

    strobe_sleep(nowMicros());
  }
  else busylooped += 1;

}