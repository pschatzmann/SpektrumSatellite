/**
 * Example Use of the SpektrumSatellite to receive the data on the RX line and send it
 * as binary data via UDP.
 * 
 * Please check and adapt the pin assignments for your Microcontroller. 
 * This demo supports an ESP32 or ESP8266
 */

#include "SpektrumSatellite.h"

#ifdef ESP32
  #include <WiFi.h>
  #include <WiFiUdp.h>
#else
#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <WiFiUdp.h>
#else
    #error "This demo requires an ESP32 or ESP8266 -> Please convert the sketch to your board"
#endif
#endif


const char* ssid = "Your SSID";                 //Change this to your router SSID.
const char* password =  "Your Password";        //Change this to your router password.
const char * udpAddress = "10.147.17.0";  //Change this to match your network
const int udpPort = 6789;                 //Change this if you need another port 
unsigned long intervall = 500;            // send every 500ms (=2 messages per second)
unsigned long intervallTime;

SpektrumSatellite<uint16_t> satellite(Serial2);
WiFiUDP udp;


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  Serial2.begin(SPEKTRUM_SATELLITE_BPS);
  satellite.setLog(Serial);

  //Initiate WIFI connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
}

void loop() {

  if (millis()>intervallTime) {
    if (satellite.getFrame()) {   
      intervallTime = millis()+intervall;
      // send the binary frame via UDP (sendData() would write to Serial2)
      udp.beginPacket(udpAddress, udpPort);
      udp.write((uint8_t*)satellite.getSendBuffer(), sizeof(Data));
      udp.endPacket();
    } 
  }
}