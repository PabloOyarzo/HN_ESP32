
#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

#include "arduino_secrets.h"

#include "heltec.h"


///////please enter your sensitive data in the Secret tab/arduino_secrets.h
/////// Wifi Settings ///////
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

WiFiMulti wifiMulti;

int initTitle[20];
int endTitle[20];

unsigned long upTime;
const int interva = 300000;

/////// Display Settings ///////
// FILL HERE WITH MODE TYPES

bool enterFirst = 0;

void setup() {
  
    // init display  
    Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
    Heltec.display->flipScreenVertically();
    Heltec.display->setFont(ArialMT_Plain_10); //10 16 24
    Heltec.display->clear();

    Serial.begin(115200);

    Serial.println();
    Serial.println();
    Serial.println();

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    wifiMulti.addAP(ssid, pass);

}


void searchTitle(String *payload){
  int payloadLength = payload->length();
  
  int titleNumber = 0;
  
  int charLocation = 0;
  int charLocation_init;
  int charLocation_end;

  for (charLocation = 0; charLocation <= payloadLength; charLocation++) {
    
    //Search first Title
    charLocation_init = charLocation + 16; // "<title><![CDATA[" = 16
    charLocation_end = charLocation + 11; // "]]></title>" = 11
    
    if (payload->substring(charLocation,charLocation_init) == "<title><![CDATA[") {
      // Found the Start of the Title
      initTitle[titleNumber] = charLocation_init;
      charLocation = charLocation_end;
    }
    if (payload->substring(charLocation,charLocation_end) == "]]></title>") {
      // Found the End of the Title
      endTitle[titleNumber] = charLocation;
      titleNumber++;
    }
  }
  
      for(int i = 0; i < 20; i++){
          Serial.println(payload->substring(initTitle[i],endTitle[i]));
      }
  
}



void displayTitle(String *payload){
    for(int i = 0; i < 20; i++){
          Heltec.display->clear();
          Heltec.display->drawStringMaxWidth(0, 0, 128, payload->substring(initTitle[i],endTitle[i]));
          Heltec.display->display();
          delay(7500);
    }
}

void loop() {
    // wait for WiFi connection
    unsigned long currentMillis = millis();
    
    if(currentMillis - upTime > 300000 || enterFirst == 0) {
      if (enterFirst == 0){
          enterFirst = 1;
      }
      upTime = currentMillis;
      if((wifiMulti.run() == WL_CONNECTED)) {
  
          HTTPClient http;
  
          Serial.print("[HTTP] begin...\n");
          http.begin("https://hnrss.org/frontpage"); //HTTP
  
          Serial.print("[HTTP] GET...\n");
          // start connection and send HTTP header
          int httpCode = http.GET();
  
          // httpCode will be negative on error
          if(httpCode > 0) {
              // HTTP header has been send and Server response header has been handled
              Serial.printf("[HTTP] GET... code: %d\n", httpCode);
  
              // file found at server
              if(httpCode == HTTP_CODE_OK) {
                  String payload = http.getString();
                  searchTitle(&payload);
                  displayTitle(&payload);
              }
          } else {
              Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
          }
  
          http.end();
      }

    }
    delay(10000);
}
