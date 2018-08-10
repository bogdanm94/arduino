/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/
#include <ArduinoJson.h>
#include <Arduino.h>
#include "rdm630.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>

#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;
DynamicJsonBuffer jsonBuffer;



rdm630 rfid(12, 14);
unsigned long previousMillis = 0;
const unsigned long interval =  60UL * 1000UL;




void setup() {

  USE_SERIAL.begin(115200);
  // USE_SERIAL.setDebugOutput(true);
  EEPROM.begin(500);
  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  rfid.begin();
  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Markovic 2.4GHz", "0216363646");

}

void loop() {
  byte data[6];
  byte length;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the time you should have toggled the LED
    previousMillis += interval;
    gettags();
  }
  //  while (Serial.available()) {
  //    unsigned long answer = Serial.parseInt();
  //    Serial.println(answer);
  //    if (answer != 0) {
  //      if (checkTag(answer))
  //        Serial.println("TAG OK");
  //      else
  //        Serial.println("TAG NOT OK");
  //    }
  //
  //  }


  if (rfid.available()) {
    rfid.getData(data, length);
    unsigned long id;
    id = data[2] * 100000  + (data[3] * 256) + data[4];
    unsigned long result =
      ((unsigned long int)data[1] << 24) +
      ((unsigned long int)data[2] << 16) +
      ((unsigned long int)data[3] << 8) +
      data[4];
    if (data != 0 ) {
      if (checkTag(id, result))
        Serial.println("TAG OK");
      else
        Serial.println("TAG NOT OK");
    }
  }
}

bool checkTag(unsigned long tag, unsigned long tag2) {

  for (int i = 0; i < EEPROM.length(); i += sizeof(unsigned long)) {
    unsigned long value;
    EEPROM.get(i, value);
    Serial.print(i);
    Serial.print("\t");
    Serial.print(value);
    Serial.println();
    if (tag == value || tag2 == value) {
      return true;
    }

  }
  return false;

}

void gettags() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    // Connect to HTTP server
    WiFiClient client;
    client.setTimeout(10000);
    if (!client.connect("markovic.duckdns.org", 80)) {
      Serial.println(F("Connection failed"));
      return;
    }

    Serial.println(F("Connected!"));

    // Send HTTP request
    client.println(F("GET /api/rfid/gettags/jkv1954 HTTP/1.0"));
    client.println(F("Host: markovic.duckdns.org"));
    client.println(F("Connection: close"));
    if (client.println() == 0) {
      Serial.println(F("Failed to send request"));
      return;
    }

    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return;
    }

    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
      Serial.println(F("Invalid response"));
      return;
    }
    Serial.println(client);
    //const size_t bufferSize = JSON_ARRAY_SIZE(6) + 6*JSON_OBJECT_SIZE(2) + 180;

    DynamicJsonBuffer jsonBuffer;

    JsonArray& root = jsonBuffer.parseArray(client);

    JsonArray& root_ = root;

    Serial.print("Size of: ");
    Serial.println(sizeof(root_));

    int addr = 0;
    for (int i = 0; i < 200; i++) {

      int root_0_id = root_[i]["id"]; // 6
      unsigned long root_0_rfid = root_[i]["rfid"]; // 11101341
      Serial.println(root_0_id);
      Serial.println(root_0_rfid);
      //String output;
      unsigned long temp;
      EEPROM.get(addr, temp);
      if (root_0_rfid != 0) {
        if ( temp != root_0_rfid ) {
          EEPROM.put(addr, root_0_rfid);
        }
      }

      //      EEPROM.update(addr, root_0_rfid);
      EEPROM.commit();
      //Serial.print("EEPROM contents at Address=12 is : ");
      //EEPROM.get(addr, output);
      //Serial.println(output);
      Serial.print("Size of string: ");
      Serial.println(sizeof(root_0_rfid));
      addr += sizeof(unsigned long);
    }
    // Disconnect
    client.stop();
  }
}

