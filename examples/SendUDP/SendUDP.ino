

/**
 * Example Use of the SpektrumSatellite class to read the data from analog lines and 
 * send it via UDP. 
 * 
 * With this we can implement a simple custom remote control which reads the joystick 
 * values and sends them via UDP.
 * 
 * This demo supports an ESP32 or ESP8266. Please check and adapt the pin assignments 
 * of your Microcontroller.
 * 
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
const int udpPort = 6789;
unsigned long intervall = 500;  // send every 500ms (=2 messages per second)
unsigned long intervallTime;
WiFiUDP udp;
uint8_t* buffer = new uint8_t[10*MAX_CHANNELS+1];

SpektrumSatellite<uint16_t> satellite(Serial); // Assing satellite to Serial (use Serial1 or Serial2 if available!)
SpektrumCSV<uint16_t> csv(',');

const int pins = 6;  // number of channels for servos
int inPins[] = {16, 5, 4, 0, 10, 9};  // analog input pins 

void setup() {
  Serial.begin(115200);

  //Initiate connection
  WiFi.disconnect(true);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
  }

  Serial.println();
  Serial.println("setup");

  // Activate the logging to the console only if SpektrumSatellite is not using Serial
  satellite.setLog(Serial);
  // we can define the requested binding mode
  satellite.setBindingMode(External_DSM2_11ms);
  
  // setup Input pins
  for (int j=0;j<pins; j++){
  	pinMode(inPins[j],INPUT);
  }
}

void loop() {

  if (millis()>intervallTime) {
    intervallTime = millis()+intervall;

    // read values from pins
    for (int j=0;j<pins; j++){
      int value = analogRead(inPins[j]);
      Channel channel = (Channel) (j+1);
      satellite.setChannelValue(channel, analogRead(inPins[j]));
    }

    // send binary data
    udp.beginPacket(udpAddress, udpPort);
    udp.write(satellite.getSendBuffer(), SEND_BUFFER_SIZE);
    udp.endPacket();
  } 
}

#endif