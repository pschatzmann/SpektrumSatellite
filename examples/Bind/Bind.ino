/**
 *  Sets the Satellite Receiver into Binding Mode:
 *  - Please use a Microcontroller which uses a 3.3V logic.
 *  - We power the Satellite from a digital GPIO Pin (e.g 5)
 *  - Connect the Control line of the receiver to the RX pin of the Microcontroller
 *  - You can (optionally) use Serial1 for logging
 * 
 *  The receiver is set into bind mode if it receives a defined number of falling signals right
 *  after the power up:  This is the reason why we need to power the receiver via a GPIO pin!
 * 
 */

#include "SpektrumSatellite.h"

SpektrumSatellite<uint16_t> satellite(Serial); // Assing satellite to Serial (use Serial1 or Serial2 if available!)
int rxPin = 3; // pin for receiving data from serial1
int powerPin = 36; // pin for powering the Satellite


void setup() {
  // Setup logging
  Serial1.begin(115200);
  Serial1.println();
  Serial1.println("setup");

  // we can define the requested binding mode
  satellite.setBindingMode(External_DSM2_11ms);

  // setup Spektrum Satellite -> wait for some data
  if (satellite.getStatus()!=Receiving) {
    satellite.startBinding(powerPin, rxPin);
    // wait forever
    satellite.waitForData();
  }
  Serial1.println("the Satellite has been bound");

}

void loop() {
  delay(1000);    
}
