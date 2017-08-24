#ifndef SONOFFSERVERALEXA_H
#define SONOFFSERVERALEXA_H
 
 // #define SER_DEBUG             // Uncomment this to disable prints and save space
  #define OTA_UPDATE              // Uncomment this to disable OTA Uptade

  #define EEPROM_SALT 10000
  #define SONOFF_BUTTON    0      // Button (D3)
  #define SONOFF_RELAY     12     // Ralais (D6)
  #define SONOFF_LED       13     // LED    (D7)
  #define SONOFF_INPUT     14     // NC     (D5)

  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
  #include <WiFiUdp.h>
  #include <functional>
  #include "switch.h"
  #include "UpnpBroadcastResponder.h"
  #include "CallbackFunction.h"
  #include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
  #include <Ticker.h>
  #include <SimpleTimer.h>
  #include <EEPROM.h>

  String GetwebPage();
  void turnOn();
  void turnOff();
  void starttimer();
  void writeTimeToEEprom(int Time);
#endif
