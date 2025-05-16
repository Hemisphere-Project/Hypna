
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "ttimer.h"
#include "libclock/fastmillis.h"

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint64_t nowOffset = 0; 
uint32_t sendOffset = 0; // offset to send time
bool nowOK = false;

volatile uint64_t actualTxTime = 0;

void (*rcvCallback)(uint32_t from, String msg) = NULL;
Ttimer* nowTimer = NULL;

// nowMillis
uint64_t nowMicros() 
{
    return fastmicros64_isr() + nowOffset;
}

// nowAdjust
void nowAdjust(uint64_t remoteMicros) 
{   
    uint64_t localMicros = nowMicros();
    int64_t delta = remoteMicros - localMicros;
    if (delta < 0) {
        Serial.printf("-delta %lld\n", delta); 
        return;
    }
    else {
        Serial.printf("+delta %lld\n", delta); 
    }
    nowOffset += delta;
    // Serial.printf("Time sync: %lld +delta %lld\n", remoteMicros, delta);
}

// nowBroadcast
void nowBroadcast(String msg) {
    if (!nowOK) return;
    esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t *)msg.c_str(), msg.length());
    if (result == ESP_OK) {
        Serial.println("Sent with success: " + msg);
    }
    else {
        Serial.println("Error sending the data");
    }
}

// nowReceive
void nowReceive(const uint8_t *mac, const uint8_t *data, int len) 
{   
    // Time sync
    if (len == sizeof(uint64_t)) 
    {
        uint64_t receivedTime;
        memcpy(&receivedTime, data, sizeof(uint64_t));
        nowAdjust(receivedTime);
    }
    else if (rcvCallback) 
    {
        // Parse
        uint32_t from = mac[5] | mac[4] << 8 | mac[3] << 16 | mac[2] << 24;
        String msg = String((char*)data).substring(0, len);
        
        // Call the callback
        rcvCallback(from, msg);
    }
}

void nowSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
    if(status == ESP_NOW_SEND_SUCCESS) {
        int32_t sendTime = nowMicros() - actualTxTime;
        if (sendOffset == 0 || sendTime < sendOffset) {
            sendOffset = sendTime;
            Serial.printf("new send Offset %ld\n", sendOffset);
        }
    }
}

// nowInit
void nowInit( void (*rcvHook)(uint32_t from, String msg) ) 
{   
    fastinit();     // init fast clock

    // APLL sync
    // rtc_clk_apll_enable(true);
    // rtc_clk_apll_coeff_set(0, 0, 0, 0); // Identical PLL coefficients

    // Set ESP32 in STA mode to begin with
    WiFi.mode(WIFI_STA);

    // Print MAC address
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());

    esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_1M_L); 

    // Initialize ESP-NOW
    if (esp_now_init() == ESP_OK)
    {
        Serial.println("ESP-NOW Init Success");
        esp_now_register_recv_cb(nowReceive);
        esp_now_register_send_cb(nowSent);
        rcvCallback = rcvHook;

        // Set up the timer to broadcast the time every second
        //
        if (nowTimer != NULL) {
            nowTimer->stop();
            delete nowTimer;
        }
        nowTimer = new Ttimer();
        nowTimer->every(1000, []() {
            if (!nowOK) return;
            actualTxTime = nowMicros(); // Reset before send
            uint64_t txTime = actualTxTime + sendOffset;
            esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&txTime, sizeof(txTime));
        });

    }
    else
    {
        Serial.println("ESP-NOW Init Failed");
        delay(3000);
        ESP.restart();
    }

    // Add Broadcast
    esp_now_peer_info_t peerInfo = {};
    memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
    if (!esp_now_is_peer_exist(broadcastAddress)) esp_now_add_peer(&peerInfo);

    nowOK = true;
}

// nowStop
void nowStop() 
{
    nowOK = false;
    esp_now_unregister_recv_cb();
    esp_now_del_peer(broadcastAddress);
    esp_now_deinit();

    delay(100);
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.mode(WIFI_STA);
}
