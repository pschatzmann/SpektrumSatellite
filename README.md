# Spectrum Satellite API for Arduino

 This project implements an Arduino API for the Spectrum Satellite Receiver. It should work with any type of board (e.g. Arduinos, ESP32, ESP8266 etc)

 - Complete Implementation of the Specification (See https://www.spektrumrc.com/ProdInfo/Files/Remote%20Receiver%20Interfacing%20Rev%20A.pdf)
 - Support for sending data
 - Support for receiving data
 - Support for all channels
 - Support for binding using different BindModes
 - Automatic handling of 1024 or 2048 servo data
 - Support of different data types and automatic scaling of channel values 
 - Optional Support of logging using a specified Serial pin
 - Provides Serialization to and from CSV format

## Usage Scenarios
The following usage scenarios are supported and documented with examples
 - Binding of a Spektrum Satellite (Bind)
 - Implement a Receiver:  Receive Serial Satellite Data and update PWN Pins (Receive)
 - Implement a Remote Control Radio: Reads Analog input Pins and sends it out in the Serial Spektrum Format  - Receive Serial Satellite Data and send it e.g. via UDP (Gateway)
 - Receive Serial Satellite Data and send it as CSV e.g. via UDP (GatewayCSV)
(SendUDP)

## Example: Implementing a Remote Control

In this Example we use of the SpektrumSatellite class to read the data from analog lines and send it via Bloothooth: With this we can implement a simple custom remote control running on a ESP32 which reads the joystick values and sends them out to a receiver.
Please check and adapt the pin assignments of your Microcontroller.
  
```
#include "BluetoothSerial.h"
#include "SpektrumSatellite.h"

const char* bt_name = "RemoteControl";
const char* bt_address = "HC-05";
const char* bt_pin = "1234";
const int  inputPins[] = {13, 12, 14, 27, 26, 25, 33, 32};  // analog input pins 
const int  numberOfPins = sizeof(inputPins)/sizeof(inputPins[0]);
const long intervall = 500;
long intervallTimeout;

BluetoothSerial SerialBT;
SpektrumSatellite<uint16_t> satellite(SerialBT);


void setup() {
  // Logging
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  // Activate the logging to the console only if SpektrumSatellite is not using Serial
  satellite.setLog(Serial);

  // connect to BT
  SerialBT.begin(bt_name, true); //Bluetooth device name
  SerialBT.setPin(bt_pin);
  SerialBT.connect(bt_address);
  while(!SerialBT.connected()) {
    Serial.println(".");
    delay(1000);
  }
  Serial.println("Connected to BT Succesfully!");

  
  // setup Input pins
  for (auto const& pin : inputPins){
    pinMode(pin,INPUT);
  }
}


void loop() {
  
  // send only every 500ms
  if (millis()>intervallTimeout) {
     intervallTimeout = millis()+intervall;

    // read values from pins
    for (int j=0;j<numberOfPins; j++){
      Channel channel = (Channel) (j+1);
      int value = analogRead(inputPins[j]);
      satellite.setChannelValue(channel, value);
    }

    // send binary data
    satellite.sendData();
  } 
  
}

```




