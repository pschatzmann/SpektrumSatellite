
/**
 * Example Use of the SpektrumSatellite class to recieve the data via UDP from the RemoteControl
 * in order to update the Servos
 * 
 * With this we can implement a simple custom remote receiver
 * 
 * This demo supports an ESP32 or ESP8266. Please check and adapt the pin assignments 
 * of your Microcontroller.
 * 
 */

#include "SpektrumSatellite.h"
#include "SpektrumCSV.h"
#include "ESP32_Servo.h"

#ifdef ESP32
  #include <WiFi.h>
  #include <WiFiUdp.h>
#else
    #error "This demo requires an ESP32 or ESP8266 -> Please convert the sketch to your board"
#endif


char* ssid = "RemoteControl";                 //Change this to your router SSID.
char* password =  "password123";        //Change this to your router password.
const int udpPort = 6789;
IPAddress gateway(192,168,4,0);
IPAddress subnet(255,255,255,0);   
IPAddress local_IP(192,168,4,2); 
WiFiUDP udp;

SpektrumSatellite<uint16_t> satellite(udp); // Assing satellite to Serial (use Serial1 or Serial2 if available!)
SpektrumCSV<uint16_t> csv;

const int pins = 6;  // number of channels for servos
int pwmPins[] = {2, 0, 4, 5, 6, 7};  // servo pins 
Servo servos[pins];  // allocate servos for all channels
uint8_t buffer[1024];


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  //Initiate WIFI Hostspot connection
  WiFi.disconnect(true);
  WiFi.config(local_IP, gateway, subnet);
 // WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println("Connected to WIFI");

  // Activate the logging to the console only if SpektrumSatellite is not using Serial
  satellite.setLog(Serial);
  satellite.setLogMod(0);
  
  // setup PWM pins
  for (int j=0;j<pins; j++){
    //servos[j].attach(pwmPins[j]);
  }

  // start to listen for data
  if (udp.begin(udpPort))
    Serial.print("Processing UPD now...");

}


void loop() {
  // Start processing the next available incoming packet
  if (udp.parsePacket()>0){
    if (satellite.getFrame()) {   
      for (int j=0;j<pins; j++){
         Channel ch = static_cast<Channel>(j);   
         long value = satellite.getChannelValue(ch);
         //servos[j].write(value);
      }  
  
      // log data as CSV to console
      csv.toString(satellite, buffer, 1024);
      satellite.sendData(buffer);   
      Serial.print((char*)buffer);       
    } 
  }
  
}
