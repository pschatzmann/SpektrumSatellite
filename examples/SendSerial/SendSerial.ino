#include "SpektrumSatellite.h"

/**
 * Example Use of the SpektrumSatellite class to read the data from analog lines and 
 * sends it out as Serial Binary data on another serial line. 
 * 
 * Please check and adapt the pin assignments for input pins of your Microcontroller.
 * If you activate the logging the Serial port used by the SpektrumSatellite must
 * not use Serial (it needs to use e,g Serial1 or Serial2
 * 
 */

SpektrumSatellite<uint16_t> satellite(Serial2); // Assing satellite to Serial (use Serial1 or Serial2 if available!)
const int pins = 6;  // number of channels for servos
int inPins[] = {16, 5, 4, 0, 10, 9};  // analog input pins 


void setup() {

  Serial2.begin(SPEKTRUM_SATELLITE_BPS);
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  // Activate the logging to the console only if SpektrumSatellite is not using Serial
  satellite.setLog(Serial);
  // we can define the requested binding mode
  satellite.setBindingMode(External_DSM2_11ms);

  //scale the values from 0 to 4095 which is the max ADC value for the ESP32
  satellite.setChannelValueRange(0.0, 4095.0);

  // setup Input pins
  for (int j=0;j<pins; j++){
  	pinMode(inPins[j],INPUT);
  }
}

void loop() {

	for (int j=0;j<pins; j++){
		int value = analogRead(inPins[j]);
		Channel channel = (Channel) (j+1);
	  satellite.setChannelValue(channel, analogRead(inPins[j]));
	}
  // send binary data
	satellite.sendData();
  
}
