
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

int initTitle[20] = {0};
int endTitle[20] = {0};

char tittles [20][150] = {0};

unsigned long lastTimeEnter = 0;
const int interval = 5*60*100; // 100ms*60sec*5min

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


int searchTitles(String *payload){
  int payloadLength = payload->length();

  int titleNumber = 0;

  int charLocation = 0;
  int charLocation_init = 0;
  int charLocation_end = 0;

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

      // Save strings on string array called Tittles
      int amountofletters = initTitle[titleNumber] - endTitle[titleNumber];
      String buffer = payload->substring(initTitle[titleNumber],endTitle[titleNumber]);
      buffer.toCharArray(tittles[titleNumber], amountofletters);

      titleNumber++;
    }
  }
  return titleNumber;
}



void displayTitle(String *payload){
    for(int i = 0; i < 20; i++){
          Heltec.display->clear();
          Heltec.display->drawStringMaxWidth(0, 0, 128, payload->substring(initTitle[i],endTitle[i]));
          Heltec.display->display();
          Serial.println(i);
          Serial.println(initTitle[i]);
          Serial.println(endTitle[i]);
          int amountofletters = endTitle[i] - initTitle[i];
          Serial.println(amountofletters);
          Serial.println("-----------------------");
          delay(7500);
    }
}

void loop() {

    // Updating Uptime
    unsigned long upTime = millis();

    // waits 5min to do search again
    if(upTime - lastTimeEnter > interval || enterFirst == 0) {
      if (enterFirst == 0){
          enterFirst = 1;
      }
      lastTimeEnter = upTime;
      if((wifiMulti.run() == WL_CONNECTED)) {

          HTTPClient http;

          Serial.print("[HTTP] begin...\n");
          http.begin("https://hnrss.org/frontpage"); //HTTP

          Serial.print("[HTTP] GET...\n");
          // start connection and send HTTP header
          int httpCode = http.GET();

          if(httpCode == HTTP_CODE_OK) {
              String payload = http.getString();
              searchTitles(&payload);
              displayTitle(&payload);
          } else {
              Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
          }

          http.end();
      }

    }
    delay(10000);
}
