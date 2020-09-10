/**
 *  Sets the Satellite Receiver into Binding Mode with an ESP32:
 *  - Please use a Microcontroller which uses a 3.3V logic.
 *  - We power the Satellite via a digital GPIO Pin (e.g 23)
 *  - We use Serial2 to connect the Data line of the Satellite. Connect the RX pin of the Microcontroller (GPIO 23)
 *  - We use Serial for logging
 * 
 *  The receiver is set into bind mode if it receives a defined number of falling signals right
 *  after the power up:  This is the reason why we need to power the receiver via a GPIO pin!
 * 
 */

#include "SpektrumSatellite.h"
#include "SpektrumCSV.h"

#define ONBOARD_LED  2

SpektrumSatellite<uint16_t> satellite(Serial2); 
SpektrumCSV<uint16_t> csv;
int rxPin = 16; 
int powerPin = 23; 

// blinking the LED
bool led_state = true;
int led_interval;
long led_timeout;

// print buffer for csv
uint8_t buffer[1024];


void setup() {
  // Setup logging
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  satellite.setLog(Serial);

  pinMode(ONBOARD_LED,OUTPUT);
  digitalWrite(ONBOARD_LED, HIGH);

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
     led_state = false;
     // write data to log
     csv.toString(satellite, buffer, 1024);
     Serial.print((char*)buffer);     
  }
  
  // update led
  if (millis()>led_timeout){
     if (led_interval>0)
         led_state = !led_state;
     led_timeout = millis()+led_interval;
     digitalWrite(ONBOARD_LED, led_state);
  }
}