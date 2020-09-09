
/**
 * Example Use of the SpektrumSatellite class to read the data from analog lines and 
 * send it via UDP. We create an Access Point that the receiver is connecting to.
 * 
 * With this we can implement a simple custom remote control which reads the joystick 
 * values and sends them via UDP.
 * 
 * This demo supports an ESP32 or ESP8266. Please check and adapt the pin assignments 
 * of your Microcontroller.
 * 
 */

#include "SpektrumSatellite.h"
#include "SpektrumCSV.h"

#ifdef ESP32
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WiFiUdp.h>
#else
    #error "This demo requires an ESP32 or ESP8266 -> Please convert the sketch to your board"
#endif


char* ssid = "RemoteControl";                
char* password =  "password123";   

IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,0);
IPAddress subnet(255,255,255,0);   
IPAddress target_IP(192,168,4,2);

const int udpPort = 6789;
unsigned long intervall = 500;  // send every 500ms (=2 messages per second)
unsigned long intervallTime;
WiFiUDP udp;
uint8_t* buffer = new uint8_t[10*MAX_CHANNELS+1];

SpektrumSatellite<uint16_t> satellite(udp); // Assigning the Sattelite to use UDP
SpektrumCSV<uint16_t> csv;

const int pins = 6;  // number of channels for servos
int inPins[] = {16, 5, 4, 0, 10, 9};  // analog input pins 

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  //Initiate connection
  WiFi.disconnect(true);
  
  WiFi.softAPConfig (local_IP,gateway,subnet);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  Serial.println("Access Point started");

  // Activate the logging to the console only if SpektrumSatellite is not using Serial
  satellite.setLog(Serial);
  satellite.setLogMod(1); // log every record
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
    udp.beginPacket(target_IP, udpPort);
    //udp.write(satellite.getSendBuffer(), SEND_BUFFER_SIZE);
    satellite.sendData();
    udp.endPacket();
  } 
}