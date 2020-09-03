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


char* ssid = "Your SSID";                 //Change this to your router SSID.
char* password =  "Your Password";        //Change this to your router password.
const char * udpAddress = "10.147.17.0";  //Change this to match your network
const int udpPort = 6789;                 //Change this if you need another port 
unsigned long intervall = 500;            // send every 500ms (=2 messages per second)
unsigned long intervallTime;
uint8_t* buffer = new uint8_t[10*MAX_CHANNELS+1];

SpektrumSatellite<float> satellite(Serial2); // we use doubles!
SpektrumCSV<float> csv(',');
WiFiUDP udp;


void setup() {
  Serial2.begin(SPEKTRUM_SATELLITE_BPS);
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  // Activate the logging to the console only if SpektrumSatellite is not using Serial
  satellite.setLog(Serial);

  //Initiate WIFI connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
  }
}

void loop() {

  if (millis()>intervallTime) {
    if (satellite.getFrame()) {   
      // send via UDP
      udp.beginPacket(udpAddress, udpPort);
      satellite.sendData();
      udp.endPacket();
    } 
  }
}