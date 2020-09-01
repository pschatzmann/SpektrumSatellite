/**
 * Example Use of the SpektrumSatellite to receive the data on the RX line and send data as CSV text via UDP.
 * 
 * Please check and adapt the pin assignments for your Microcontroller. We use this example to send
 * the data to Flightgear: Flighger expects float values berween -1.0 and 1.0. Therefore we use
 * define SpektrumSatellite<float>  and set the range with satellite.setChannelValueRange(-1.0f, 1.0f);
 * 
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

SpektrumSatellite<float> satellite(Serial); // we use doubles!
SpektrumCSV<float> csv(',');
WiFiUDP udp;


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  satellite.setLog(Serial);

  //Initiate WIFI connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
  }

  // we can define the requested binding mode
  satellite.setBindingMode(External_DSM2_11ms);

  //scale the values from 0 to -1.0 to 1.0
  satellite.setChannelValueRange(-1.0f, 1.0f);

}

void loop() {

  if (millis()>intervallTime) {
    if (satellite.getFrame()) {   
      // send CSV via UDP
      csv.toString(satellite, buffer, sizeof(buffer));
      int len = strlen((char*)buffer);

      udp.beginPacket(udpAddress, udpPort);
      udp.write(buffer, len);
      udp.endPacket();
    } 
  }
}