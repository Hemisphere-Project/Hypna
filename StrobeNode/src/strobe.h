#include <Arduino.h>
#include "librmt/pixel.h"

// Variables
float strobeFreq=1;
int duty=0;
uint32_t strobePeriod, delayON, delayOFF, nextSwitch = 0;
bool state = false;

uint32_t lastSettingsTS = 1;

pixelColor_t outputON  = pixelFromRGBW(0, 0, 0, 0);
pixelColor_t outputOFF = pixelFromRGBW(0, 0, 0, 0);

// Settings
void strobe_set(
      float Freq, // Hz 
      int Duty,   // % 
      int Master1, int Red, int Green, int Blue, int White // 0-255
  ) {

  if (Freq <= 0) Freq = 1;
  strobeFreq = Freq; 
  strobePeriod = 1000000 / Freq; // µs
  duty = Duty;
  delayON = strobePeriod * Duty / 100;
  delayOFF = strobePeriod - delayON;
  outputON = pixelFromRGBW(Green * Master1 / 255, Red * Master1 / 255, Blue * Master1 / 255, White * Master1 / 255);
  state = false;

  Serial.printf("Strobe settings: %fHz, %d%%, %d, %d, %d, %d, %d\n", Freq, Duty, Master1, Red, Green, Blue, White);
  lastSettingsTS = nowMillis();
}

void strobe_set (
      uint32_t ts, // µs    
      float Freq, // Hz 
      int Duty,   // % 
      int Master1, int Red, int Green, int Blue, int White // 0-255
  ) {
    if (ts > lastSettingsTS+2000 || ts == 0) {
      strobe_set(Freq, Duty, Master1, Red, Green, Blue, White);
      if (ts > lastSettingsTS) lastSettingsTS = ts;
    }
    else {
        // Serial.printf("Remote setting too old, discarding... %lld\n", ts-lastSettingsTS);
    }
}

// Update state -> true if state changed
bool strobe_update(uint64_t unow) {
  int p = unow % strobePeriod;
  
  // Set ON
  if (p < delayON && state == false) {
    state = true;
    nextSwitch = unow + delayON - p;
    if (p > 50) Serial.printf("WARNING: late ON: +%dµs\n", p);
    return true;
  }
  
  // Set OFF
  if (p > delayON && state == true) {
    state = false;
    nextSwitch = unow + delayON + delayOFF - p;
    p = p-delayON;
    if (p > 50) Serial.printf("WARNING: late OFF: +%dµs\n", p);
    return true;
  }
  
  return false;
}



// Get State
bool strobe_state() {
  return state;
}

// Get output
pixelColor_t strobe_get() {
  if (state) return outputON;
  else return outputOFF;
}

// Sleep
const int BUSYTIME = 80; // µs
void strobe_sleep(uint64_t unow) {
    int32_t sleepTime = nextSwitch - unow - BUSYTIME;
    if (sleepTime > 0) delayMicroseconds(sleepTime);
    else Serial.printf("WARNING: no sleep time... %dµs\n", sleepTime);
}

// Values
float strobe_getFreq() {
  return strobeFreq;
}
int strobe_getDuty() {
  return duty;
}
int strobe_getMaster1() {
  return outputON.g;
}
int strobe_getRed() {
  return outputON.r;
}
int strobe_getGreen() {
  return outputON.g;
}
int strobe_getBlue() {
  return outputON.b;
}
int strobe_getWhite() {
  return outputON.w;
}
uint64_t strobe_getTS() {
  return lastSettingsTS;
}