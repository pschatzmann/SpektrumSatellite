/**
 * Example Use of the SpektrumSatellite to receive data:
 * 
 * Please check and adapt the pin assignments for your Microcontroller.
 * If you activate the logging the Serial port used by the SpektrumSatellite must
 * not use Serial (it needs to use e,g Serial1 or Serial2
 * You also might comment out the automatic binding if there is no data because
 * the Receiver needs to be bound only once! If you use the binding please make sure 
 * that you indicate the correct rxPin which is assosiated with the RX pin of the used
 * Serial interface. 
 * 
 */

#include "SpektrumSatellite.h"

#ifdef ESP32
#include <ESP32_Servo.h>
#else
#include "Servo.h"
#endif

SpektrumSatellite<uint16_t> satellite(Serial2); // Assing satellite to Serial (use Serial1 or Serial2 if available!)
const int pins = 6;  // number of channels for servos
Servo servos[pins];  // allocate servos for all channels
int pwmPins[] = {16, 5, 4, 0, 10, 9};  // servo pins 
int rxPin = 3; // pin for receiving data from serial1


void setup() {
  Serial2.begin(SPEKTRUM_SATELLITE_BPS);
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  // Activate the loggin to the console only if SpektrumSatellite is not using Serial
  satellite.setLog(Serial);
  // we can define the requested binding mode
  satellite.setBindingMode(External_DSM2_11ms);

  //scale the values from 0 to 180 degrees for PWM
  satellite.setChannelValueRange(0, 180);

  // wait forever for data
  satellite.waitForData();
  
  // setup PWM pins
  for (int j=0;j<pins; j++){
    servos[j].attach(pwmPins[j]);
  }
}

void loop() {
  
  if (satellite.getFrame()) {   
    for (int j=0;j<pins; j++){
       Channel ch = static_cast<Channel>(j);   
       long value = satellite.getChannelValue(ch);
       servos[j].write(value);
    }        
  }   

}