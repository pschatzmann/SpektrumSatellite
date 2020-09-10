/**
 *  Sets the Satellite Receiver into Binding Mode:
 *  - Please use a Microcontroller which uses a 3.3V logic.
 *  - We power the Satellite from a digital GPIO Pin (e.g 5)
 *  - Connect the Control line of the receiver to the RX pin of the Microcontroller
 *  - You can (optionally) use Serial1 for logging
 *  - This example has been written for a ESP32
 * 
 *  The receiver is set into bind mode if it receives a defined number of falling signals right
 *  after the power up:  This is the reason why we need to power the receiver via a GPIO pin!
 * 
 */

#include "SpektrumSatellite.h"
#define ONBOARD_LED  2

SpektrumSatellite<uint16_t> satellite(Serial); 
int rxPin = 3; 
int powerPin = 23; 

// blinking the LED
bool led_state = true;
int led_interval;
long led_timeout;

void setup() {
  // Setup logging
  Serial1.begin(115200);
  Serial1.println();
  Serial1.println("setup");

  // we set the receiver into binding mode
  satellite.setBindingMode(External_DSM2_11ms);
  satellite.startBinding(powerPin, rxPin);
 
  // start blinking
  led_interval = 1000;

  // switch rxPin to receive data on Serial 
  Serial.begin(SPEKTRUM_SATELLITE_BPS);

}

void loop() {
  // when we receive data we 
  if (satellite.getFrame()) {  
     led_interval = 0;
     led_state = true;
  }
  
  // update led
  if (millis()>led_timeout){
     if (led_interval>0)
         led_state = !led_state;
     led_timeout = millis()+led_interval;
     digitalWrite(ONBOARD_LED, led_state);
  }
}