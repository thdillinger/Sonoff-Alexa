/* Th.Dillinger, www.dillinger-engineering.de 
 * Diese Vorlage beinhaltet die folgenden Optionen 
 * Captiv Portal für die Anmeldung an einen lokalen WLAN Router wenn die automatische Verbindung scheitert
 * OTA Update
 * Anmeldung an Amazone Alexa und und einem Webbrowser 

   4MB/512SPIFFS flash sizee Memory replaced

   Sonoff Programming interface
   1 - vcc 3v3
   2 - rx
   3 - tx
   4 - gnd
   5 - gpio 14

   esp8266 connections
   gpio  0 - button
   gpio 12 - relay
   gpio 13 - green led - active low
   gpio 14 - pin 5 on header

*/
#include "SonoffServerAlexa.h"

/* MDNSResponder is neded for
 *  For Mac OSX support is built in through Bonjour already.
   For Linux, install Avahi.
   For Windows, install Bonjour. */
//MDNSResponder mdns;    

// Instanz for Alexa 
Switch *sonoff = NULL;
UpnpBroadcastResponder upnpBroadcastResponder;

char *Hostname = "New SonOff";
static bool BLYNK_ENABLED = true;
int OnTime = 5;
unsigned TimeCounter;

const char RESPONSE_HEAD1[]  PROGMEM = "<html><head><title>";
const char RESPONSE_HEAD2[]  PROGMEM = "</title></head>";
const char RESPONSE_BODY[]   PROGMEM = "<body><font color=\"#000000\"><body bgcolor=\"#d0d0f0\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
const char RESPONSE_TITEL1[] PROGMEM = "<h1><u>Sonoff Timerswitch - "; 
const char RESPONSE_TITEL2[] PROGMEM = "</u></h1>"; 
const char RESPONSE_FORM[]   PROGMEM = "<div><a href=\"on\"><button>ON</button></a>&nbsp;<a href=\"off\"><button>OFF</button></a>&nbsp;<a href=\"timer\"><button>Start Timer</button></a></div></p>";
const char RESPONSE_END[]    PROGMEM = "<BR><BR><FONT SIZE=-1><hr/><a href=\"http://www.dillinger-engineering.de\" target=\"_blank\">www.dillinger-engineering.de</a><BR></body></html>";

typedef struct {
  int   salt = EEPROM_SALT;
  char  deviceName[33]      = "New SonOff"; // default Modulname
  char  invocationname[33]  = "Invocation Name"; // Alexa invocationname
} WMSettings;
WMSettings settings;

//needed for OTA Update
#ifdef OTA_UPDATE
  #include <WiFiUdp.h>
  #include <ArduinoOTA.h>
#endif

Ticker ticker;       // For LED status
SimpleTimer timer;   // Golbal Instans fo SimpleTimer called timer

const int CMD_WAIT = 0;
const int CMD_BUTTON_CHANGE = 1;

int cmd = CMD_WAIT;
int relayState = LOW;

//inverted button state
int buttonState = HIGH;

static long startPress = 0;

String GetwebPage(){
  String webPage = String(RESPONSE_HEAD1) + String(Hostname) + String(RESPONSE_HEAD2) + String(RESPONSE_BODY) + String(RESPONSE_TITEL1)+ String(settings.invocationname) + String(RESPONSE_TITEL2) + String(RESPONSE_FORM);
  //  Auswahl der Wartezeit

  webPage += "<form id=\"FSwitch\">";
  webPage += "<label for=\"time\">Time:";
  webPage += "<input name=\"time\" id=\"time\" type=\"number\" min=\"1\" max=\"1440\" step=\"1\" value=\"";
  webPage += OnTime;
  webPage += "\"> Min.&nbsp;";
  webPage += "<button type=\"submit\" form=\"FSwitch\" value=\"Submit\">Submit</button></form>";
  webPage += "<BR><BR>Switch is: ";
  if(relayState == HIGH){
    webPage +=" ON";
  }else{
    webPage +=" OFF";
  }
  webPage += String(RESPONSE_END);  
  return(webPage);
}

void tick()
{
  //toggle state
  int state = digitalRead(SONOFF_LED);  // get the current state of GPIO1 pin
  digitalWrite(SONOFF_LED, !state);     // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  #ifdef SER_DEBUG
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
    //if you used auto generated SSID, print it
    Serial.println(myWiFiManager->getConfigPortalSSID());
    //entered config mode, make led toggle faster
  #endif
  ticker.attach(0.15, tick);  // schenlles Blinken der LED
}

void setState(int s) {
  digitalWrite(SONOFF_RELAY, s);
  digitalWrite(SONOFF_LED, (s + 1) % 2); // led is active low (also invertiert, ist s=0 dann ist s+1=1 mod 2 = 0)
  #ifdef SER_DEBUG
    Serial.println(relayState);
  #endif
}

void turnOn() {
  #ifdef SER_DEBUG
    Serial.print("relais state On: ");
  #endif
  ticker.detach();
  TimeCounter = 0;
  relayState = HIGH;
  setState(relayState);
}

void turnOff() {
  #ifdef SER_DEBUG
    Serial.print("relais state Off: ");
  #endif  
  ticker.detach();
  TimeCounter = 0;
  relayState = LOW;
  setState(relayState);
}

void toggle() {
  #ifdef SER_DEBUG
    Serial.print("toggle state: ");
  #endif
  ticker.detach();
  TimeCounter = 0;
  relayState = relayState == HIGH ? LOW : HIGH;
  setState(relayState);
}

void starttimer(){
  turnOn();  // Relais ein
  TimeCounter = OnTime * 60; // Zeit in Sekunden
  ticker.attach(2, tick);    // langsames 2 Sek. Blinken der LED
  #ifdef SER_DEBUG
    Serial.println("Timer stopp");
  #endif  
}

void onChangeInt(){
  cmd = CMD_BUTTON_CHANGE;
}

void writeTimeToEEprom(int Time){
  OnTime = Time;  
  EEPROM.begin(512);
  EEPROM.put(500, Time);
  EEPROM.end();
  #ifdef SER_DEBUG
    Serial.println(Time);
  #endif
}

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  #ifdef SER_DEBUG
    Serial.println("Should save config");
  #endif
  shouldSaveConfig = true;
}

void reset() {
  //reset settings to defaults 
    WMSettings defaults;
    settings = defaults;
    writeTimeToEEprom(5);  // Default 5 Minutes
  //reset wifi credentials
  WiFi.disconnect();
  delay(1000);
  ESP.reset();
  delay(1000);
}

void restart() {
  ESP.reset();
  delay(1000);
}

/************************************* SETUP ****************************************************************/
void setup(){
  #ifdef SER_DEBUG
    Serial.begin(115200);
  #endif

  //set led pin as output
  pinMode(SONOFF_LED, OUTPUT);
  // start ticker with 0.6 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  WiFiManager wifiManager;
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //timeout - this will quit WiFiManager if it's not configured in 3 minutes, causing a restart
  wifiManager.setConfigPortalTimeout(180);

  //custom params
  EEPROM.begin(512);
  EEPROM.get(0, settings);
  EEPROM.end();

  if (settings.salt != EEPROM_SALT) {
    #ifdef SER_DEBUG
      Serial.println("Invalid settings in EEPROM, trying with defaults");
    #endif
    WMSettings defaults;
    settings = defaults;
  }
  Hostname = settings.deviceName; 
  #ifdef SER_DEBUG
    Serial.println(WiFi.localIP());
    Serial.println("Alexa Invocation Name:");
    Serial.println(settings.invocationname);
  #endif
 
  WiFiManagerParameter custom_devicename_text("Device name");
  wifiManager.addParameter(&custom_devicename_text);

  WiFiManagerParameter custom_devicename("device-name", "device name", settings.deviceName, 33);
  wifiManager.addParameter(&custom_devicename);

  WiFiManagerParameter custom_invocation_text("Alexa Invocation Name:");
  wifiManager.addParameter(&custom_invocation_text);

  WiFiManagerParameter custom_invocationname("invocation-name", "invocation name", settings.invocationname, 33);
  wifiManager.addParameter(&custom_invocationname);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  if (!wifiManager.autoConnect(Hostname)) {
    #ifdef SER_DEBUG
      Serial.println("failed to connect and hit timeout");
    #endif
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    strcpy(settings.deviceName, custom_devicename.getValue());
    strcpy(settings.invocationname, custom_invocationname.getValue());
    #ifdef SER_DEBUG
      Serial.println("Saving config");
      Serial.print("Device Name: ");
      Serial.println(settings.deviceName);
      Serial.print("Invocation Name: ");
      Serial.println(settings.invocationname);
    #endif
    EEPROM.begin(512);
    EEPROM.put(0, settings);
    EEPROM.end();
  }

  #ifdef OTA_UPDATE
    //OTA -Update
    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);
    // Hostname defaults to esp8266-[ChipID] 
    ArduinoOTA.setHostname(Hostname);

    // No authentication by default
    //ArduinoOTA.setPassword((const char *)"xc486u");

    ArduinoOTA.onStart([]() {
      #ifdef SER_DEBUG
        Serial.println("Start");
      #endif  
    });
    ArduinoOTA.onEnd([]() {
      #ifdef SER_DEBUG
        Serial.println("\nEnd");
      #endif  
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      #ifdef SER_DEBUG
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      #endif  
    });
    ArduinoOTA.onError([](ota_error_t error) { 
      #ifdef SER_DEBUG
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)         Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)   Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)     Serial.println("End Failed");
      #endif
    });
    ArduinoOTA.begin();
    #ifdef SER_DEBUG
      Serial.println("OTA started and ready");
    #endif  
  #endif 

  //if you get here you have connected to the WiFi
  
  #ifdef SER_DEBUG
    Serial.println("connected... :)");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Device Name: ");
    Serial.println(settings.deviceName);
    Serial.print("Invocation Name: ");
    Serial.println(settings.invocationname);
  #endif

/*  if (mdns.begin(Hostname, WiFi.localIP())) {
    #ifdef SER_DEBUG
      Serial.println("MDNS responder started");
    #endif 
  }*/
  
  ticker.detach();

  // if wifi connectet start upnp BroadcastResponder to connect with Alexa   
  upnpBroadcastResponder.beginUdpMulticast();
    
  // Define your switches here. Max 14
  // Format: Alexa invocation name, local port no, on callback, off callback
  sonoff = new Switch(settings.invocationname, 80, turnOn, turnOff);

  #ifdef SER_DEBUG
    Serial.println("Adding switches upnp broadcast responder");
    Serial.print("To invocation say: ");
    Serial.println(settings.invocationname);
  #endif
  // add device for broadcast
  upnpBroadcastResponder.addDevice(*sonoff);
      
  // timed actions setup
  timer.setInterval(1000, DecTime);                    // One timer is triggered every 1000 ms
  //timer.setTimeout(10000, OnceOnlyTask);             // Another timer is set to trigger only once after 10 seconds
  //timer.setTimer(1200, TenTimesTask, 10);            // Another timer is set to trigger 10 times

  //setup button
  pinMode(SONOFF_BUTTON, INPUT);
  attachInterrupt(SONOFF_BUTTON, onChangeInt, CHANGE);

  //setup relay
  pinMode(SONOFF_RELAY, OUTPUT);
  turnOff();  // Relais aus
  
  //custom params
  EEPROM.begin(512);
  EEPROM.get(500, OnTime);
  EEPROM.end();
  if(OnTime<0){
    OnTime=5;
  }
   #ifdef SER_DEBUG
     Serial.print("Timer Zeit: "); 
     Serial.println(OnTime); 
   #endif
}

void DecTime(){
  if(TimeCounter!=0){ 
    TimeCounter --;     
      #ifdef SER_DEBUG
        Serial.print("Timer Time: ");
        Serial.println(TimeCounter);
      #endif
    if(TimeCounter==1){
      ticker.detach();
      turnOff();  // Relais aus
      #ifdef SER_DEBUG
        Serial.println("Timer stopp");
      #endif  
    }
  }
}

/************************************* LOOP ****************************************************************/
void loop(){
  //ota loop
  #ifdef OTA_UPDATE 
    ArduinoOTA.handle();
  #endif  

  timer.run();
 
  // handle upnp for Alexa 
  upnpBroadcastResponder.serverLoop();
  sonoff->serverLoop();
  
  switch (cmd) { // cmd = CMD_WAIT
    case CMD_WAIT: 
      break;
    case CMD_BUTTON_CHANGE:  
      int currentState = digitalRead(SONOFF_BUTTON);
      if (currentState == LOW && currentState != buttonState){
        startPress = millis();
        buttonState = currentState;
      }

      if(currentState == HIGH){
        cmd = CMD_WAIT;
        buttonState = currentState;
        long duration = millis() - startPress;
        #ifdef SER_DEBUG
          Serial.print("Duration: ");
          Serial.println(duration);
        #endif      
        if (duration > 8000) {
          #ifdef SER_DEBUG
            Serial.println("long press - reset settings");
          #endif  
          reset();
        }else if (duration > 4000) {
          #ifdef SER_DEBUG
            Serial.println("medium press - reset");
          #endif
          restart();
        }else if (duration > 50) {
          #ifdef SER_DEBUG
            Serial.println("short press - toggle relay");
          #endif  
          toggle(); 
        }
      }
      break;
  } // end Switch
}


