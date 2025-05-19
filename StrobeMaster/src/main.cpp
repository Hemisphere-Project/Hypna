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
const char* ssid = "HYPNA-STROBE-MASTER";
const char* password = "HypnaStrobe";

#include "usbmidi.h"
#include "nowlink.h"
#include "strobe.h"
#include "webserver.h"


void setup() {
  Serial.begin(115200);

  // Strobe init
  strobe_set(1, 10, 255, 255, 255, 255, 255); // 1Hz, 10% duty, Master 255, RGBW 255

  // ESP-NOW timesync
  nowInit(
    // RECEIVE CALLBACK
    //
    [](uint32_t from, String msg) {}, 

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

  usbmidi_init();
}

void loop() {}
