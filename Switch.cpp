#include "Switch.h"
#include "CallbackFunction.h"
#include "SonoffServerAlexa.h"
//<<constructor>>
Switch::Switch(){
    #ifdef SER_DEBUG
      Serial.println("default constructor called");
    #endif
}
//Switch::Switch(String alexaInvokeName,unsigned int port){
Switch::Switch(String alexaInvokeName, unsigned int port, CallbackFunction oncb, CallbackFunction offcb){
    uint32_t chipId = ESP.getChipId();
    char uuid[64];
    sprintf_P(uuid, PSTR("38323636-4558-4dda-9188-cda0e6%02x%02x%02x"),
          (uint16_t) ((chipId >> 16) & 0xff),
          (uint16_t) ((chipId >>  8) & 0xff),
          (uint16_t)   chipId        & 0xff);
    
    serial = String(uuid);
    persistent_uuid = "Socket-1_0-" + serial+"-"+ String(port);
        
    device_name = alexaInvokeName;
    localPort = port;
    onCallback = oncb;
    offCallback = offcb;
    
    startWebServer();
}


 
//<<destructor>>
Switch::~Switch(){/*nothing to destruct*/}

void Switch::serverLoop(){
    if (server != NULL) {
        server->handleClient();
        delay(1);
    }
}

void Switch::startWebServer(){
  server = new ESP8266WebServer(localPort);

// Server Handler
  server->on("", [&]() {
    handleRoot();
  });
 
  server->on("/", [&]() {
    handleRoot();
  });

  server->on("/setup.xml", [&]() {
    handleSetupXml();
  });

  server->on("/upnp/control/basicevent1", [&]() {
    handleUpnpControl();
  });

  server->on("/eventservice.xml", [&]() {
    handleEventservice();
  });

  server->on("/on", [&](){
    handleEventOn();
  });
  
  server->on("/off", [&](){
    handleEventOff();
  });

  server->on("/timer", [&](){
    handleEventTimer();
  });

  server->on("/time", [&](){
    handleEventTime();
  });

  //server->onNotFound(handleNotFound);
  server->begin();

  #ifdef SER_DEBUG
    Serial.print("WebServer started on port: ");
    Serial.println(localPort);
  #endif
}
 
void Switch::handleEventservice(){
  #ifdef SER_DEBUG
    Serial.println(" ########## Responding to eventservice.xml ... ########\n");
  #endif
  String eventservice_xml = "<?scpd xmlns=\"urn:Belkin:service-1-0\"?>"
        "<actionList>"
          "<action>"
            "<name>SetBinaryState</name>"
            "<argumentList>"
              "<argument>"
                "<retval/>"
                "<name>BinaryState</name>"
                "<relatedStateVariable>BinaryState</relatedStateVariable>"
                "<direction>in</direction>"
              "</argument>"
            "</argumentList>"
             "<serviceStateTable>"
              "<stateVariable sendEvents=\"yes\">"
                "<name>BinaryState</name>"
                "<dataType>Boolean</dataType>"
                "<defaultValue>0</defaultValue>"
              "</stateVariable>"
              "<stateVariable sendEvents=\"yes\">"
                "<name>level</name>"
                "<dataType>string</dataType>"
                "<defaultValue>0</defaultValue>"
              "</stateVariable>"
            "</serviceStateTable>"
          "</action>"
        "</scpd>\r\n"
        "\r\n";
          
    server->send(200, "text/plain", eventservice_xml.c_str());
}
 
void Switch::handleUpnpControl(){
  #ifdef SER_DEBUG
    Serial.println("########## Responding to  /upnp/control/basicevent1 ... ##########");      
  #endif
  
  //for (int x=0; x <= HTTP.args(); x++) {
  //  Serial.println(HTTP.arg(x));
  //}

  String request = server->arg(0);      
  #ifdef SER_DEBUG
    Serial.print("request:");
    Serial.println(request);
  #endif  

  if(request.indexOf("<BinaryState>1</BinaryState>") > 0) {
      #ifdef SER_DEBUG
        Serial.println("Got Turn on request");
      #endif
      onCallback();
  }

  if(request.indexOf("<BinaryState>0</BinaryState>") > 0) {
      #ifdef SER_DEBUG
        Serial.println("Got Turn off request");
      #endif  
      offCallback();
  }
  
  server->send(200, "text/plain", "");
}

void Switch::handleSetupXml(){
  #ifdef SER_DEBUG
    Serial.println(" ########## Responding to setup.xml ... ########\n");
  #endif  
  
  IPAddress localIP = WiFi.localIP();
  char s[16];
  sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
  
  String setup_xml = "<?xml version=\"1.0\"?>"
        "<root>"
         "<device>"
            "<deviceType>urn:Belkin:device:controllee:1</deviceType>"
            "<friendlyName>"+ device_name +"</friendlyName>"
            "<manufacturer>Belkin International Inc.</manufacturer>"
            "<modelName>Emulated Socket</modelName>"
            "<modelNumber>3.1415</modelNumber>"
            "<UDN>uuid:"+ persistent_uuid +"</UDN>"
            "<serialNumber>221517K0101769</serialNumber>"
            "<binaryState>0</binaryState>"
            "<serviceList>"
              "<service>"
                  "<serviceType>urn:Belkin:service:basicevent:1</serviceType>"
                  "<serviceId>urn:Belkin:serviceId:basicevent1</serviceId>"
                  "<controlURL>/upnp/control/basicevent1</controlURL>"
                  "<eventSubURL>/upnp/event/basicevent1</eventSubURL>"
                  "<SCPDURL>/eventservice.xml</SCPDURL>"
              "</service>"
          "</serviceList>" 
          "</device>"
        "</root>\r\n"
        "\r\n";
        
    server->send(200, "text/xml", setup_xml.c_str());
    
    #ifdef SER_DEBUG
      Serial.print("Sending :");
      Serial.println(setup_xml);
    #endif
}

void Switch::handleRoot(){
    HandleParameter();
    server->send(200, "text/html", GetwebPage());    
}

void Switch::handleEventOn(){
    turnOn();
    HandleParameter();
    server->send(200, "text/html", GetwebPage());
    delay(1000);
}

void Switch::handleEventOff(){
    turnOff();
    HandleParameter();
    server->send(200, "text/html", GetwebPage());
    delay(1000);  
}

void Switch::handleEventTimer(){
    HandleParameter();
    server->send(200, "text/html", GetwebPage());
    delay(1000); 
    starttimer();
}

void Switch::handleEventTime(){
    HandleParameter();
    server->send(200, "text/html", GetwebPage());
    delay(1000); 
}

void Switch::HandleParameter(){
  if (server->hasArg("time")){
    int Time = server->arg("time").toInt();
    writeTimeToEEprom(Time);
  } 
}
 
String Switch::getAlexaInvokeName() {
    return device_name;
}

void Switch::respondToSearch(IPAddress& senderIP, unsigned int senderPort) {
  #ifdef SER_DEBUG
    Serial.println("");
    Serial.print("Sending response to ");
    Serial.println(senderIP);
    Serial.print("Port : ");
    Serial.println(senderPort);
  #endif
  IPAddress localIP = WiFi.localIP();
  char s[16];
  sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

  String response = 
       "HTTP/1.1 200 OK\r\n"
       "CACHE-CONTROL: max-age=86400\r\n"
       "DATE: Sat, 26 Nov 2016 04:56:29 GMT\r\n"
       "EXT:\r\n"
       "LOCATION: http://" + String(s) + ":" + String(localPort) + "/setup.xml\r\n"
       "OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
       "01-NLS: b9200ebb-736d-4b93-bf03-835149d13983\r\n"
       "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
       "ST: urn:Belkin:device:**\r\n"
       "USN: uuid:" + persistent_uuid + "::urn:Belkin:device:**\r\n"
       "X-User-Agent: redsonic\r\n\r\n";

  UDP.beginPacket(senderIP, senderPort);
  UDP.write(response.c_str());
  UDP.endPacket();                    
  #ifdef SER_DEBUG
    Serial.println("Response sent !");
  #endif
}
