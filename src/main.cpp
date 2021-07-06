// Program to demonstrate the MD_Parola library
//
// Simplest program that does something useful - Hello World!
//
// MD_MAX72XX library can be found at https://github.com/MajicDesigns/MD_MAX72XX
//

#include <Arduino.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <ezTime.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "font_wide_digits.h"

#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES 6

#define CLK_PIN 14  // or SCK
#define DATA_PIN 13 // or MOSI
#define CS_PIN 2    // or SS

// Hardware SPI connection
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary output pins
// MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

#define WIFI_SSID "AR"
#define WIFI_PASS "052593062186"
Timezone local_tz;

void ota_setup()
{
    /* OTA setup */
    ArduinoOTA.setHostname("esp8266-bfc");

    ArduinoOTA.onStart([]()
        {
            Serial.println("OTA Update Start");
            P.setFont(NULL);
            P.displayClear(0);
            P.displayText("OTA Start", PA_CENTER, P.getSpeed(), P.getPause(),
                            PA_PRINT, PA_NO_EFFECT);
            while(!P.displayAnimate()) yield();
        });
    ArduinoOTA.onEnd([]()
        {
            Serial.println("\nOTA Update Done.");
            P.displayClear(0);
            P.displayText("OTA Done", PA_CENTER, P.getSpeed(), P.getPause(),
                        PA_PRINT, PA_NO_EFFECT);
            while(!P.displayAnimate()) yield();

        });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
        { 
            char dsp_msg[8];
            snprintf(dsp_msg, 16, "OTA: %u%%", (progress / (total / 100)));

            Serial.printf("Progress: %u%%\r", (progress / (total / 100))); 
            P.setFont(NULL);
            P.displayText(dsp_msg, PA_LEFT, P.getSpeed(), P.getPause(), 
                          PA_PRINT, PA_NO_EFFECT);
            P.displayAnimate();
        });
    ArduinoOTA.onError([](ota_error_t error)
        {
            Serial.printf("Error[%u]: ", error);
            P.setFont(NULL);

            P.displayClear(0);
            if (error == OTA_AUTH_ERROR)
            {
                Serial.println("Auth Failed");
                P.displayText("Auth Fail", PA_CENTER, P.getSpeed(), P.getPause(), 
                              PA_PRINT, PA_NO_EFFECT);
                while(!P.displayAnimate()) yield();

            }
            else if (error == OTA_BEGIN_ERROR)
            {

                Serial.println("Begin Failed");
                P.displayText("Begin Fail", PA_CENTER, P.getSpeed(), P.getPause(), 
                              PA_PRINT, PA_NO_EFFECT);
            } 
            else if (error == OTA_CONNECT_ERROR)
            {

                Serial.println("Connect Failed");
                P.displayText("Conn Fail", PA_CENTER, P.getSpeed(), P.getPause(), 
                              PA_PRINT, PA_NO_EFFECT);
            }
            else if (error == OTA_RECEIVE_ERROR)
            {

                Serial.println("Receive Failed");
                P.displayText("RX Fail", PA_CENTER, P.getSpeed(), P.getPause(), 
                              PA_PRINT, PA_NO_EFFECT);
            } 
            else if (error == OTA_END_ERROR)
            {

                Serial.println("End Failed");
                P.displayText("End Fail", PA_CENTER, P.getSpeed(), P.getPause(), 
                              PA_PRINT, PA_NO_EFFECT);
            }
            while(!P.displayAnimate()) yield();
        });
    ArduinoOTA.begin();

}


void setup(void)
{
    Serial.begin(115200);
    Serial.println("Hello from BFC");

    delay(1000);
    P.begin();
    P.setIntensity(1);
    P.displayReset(0);
    P.setSpeed(40);

    Serial.print("Connecting to WiFi...");

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    P.displayText("Connecting To Wifi...", PA_CENTER, P.getSpeed(), 
                  P.getPause(), PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    /* Wait for WiFi to connect */
    while (WiFi.status() != WL_CONNECTED)
    { 
        P.displayAnimate();
        yield(); /* stop ESP8266 WDT from firing */
    }
    Serial.println("Connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    char wifi_ip[24];
    snprintf(wifi_ip, 24, "IP: %s", WiFi.localIP().toString().c_str());
    P.displayClear(0);
    P.displayText(wifi_ip, PA_LEFT, P.getSpeed(),
                  P.getPause(), PA_SCROLL_LEFT, PA_SCROLL_LEFT);

    /* print the whole IP */
    while(!P.displayAnimate()) yield();


    ota_setup();

    /* NTP stuff */
    waitForSync();
    setInterval(60);
    setDebug(INFO);

    Serial.println();
    Serial.println("UTC:             " + UTC.dateTime("H:i:s"));


    // Provide official timezone names
    // https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
    local_tz.setLocation(F("America/Chicago"));

    Serial.print(F("Huntsville:     "));
    Serial.println(local_tz.dateTime("H:i:s"));

    P.setFont(font_wide_digits);

}

void loop(void)
{
    static uint32_t t_last_update = 0;
    static uint8_t last_upd_sec = 0;
    uint32_t t_now = millis();
    uint8_t  this_sec = local_tz.second(TIME_NOW);


    events();
    ArduinoOTA.handle();
    
  
    if (this_sec != last_upd_sec)
    {
        char time[9];
        local_tz.dateTime("H:i:s").toCharArray(time, 9);
        P.displayText(time, PA_LEFT, P.getSpeed(), P.getPause(), 
                      PA_PRINT, PA_NO_EFFECT);
        P.displayAnimate();
        last_upd_sec = this_sec;
    }

    t_last_update = t_now;
}
