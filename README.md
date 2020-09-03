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

## Basic Syntax
The SpektrumSatellite class expects a Stream (HardwareSerial, SoftwareSerial, UDP etc) as parameter.  You need to make sure that you set the exected baud rate (e.g. with Serial.begin(SPEKTRUM_SATELLITE_BPS)). 


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
## Example: Implementing the Receiver on the RC Airplane

In the following Example we use the SpektrumSatellite class on an Arduino Nano with the HC-05 Bluetooth module to receive the  data in order to update the servos. 

We use SoftwareSerial instead of the built in Serial interface so that we can keep the logging functionality to the console.

```
#include <SoftwareSerial.h>
#include "SpektrumSatellite.h"
#include "Servo.h"

SoftwareSerial SpektrumSerial(12, 13); // connect TX only after checking the voltage!
SpektrumSatellite<uint16_t> satellite(SpektrumSerial); // Assing satellite to Serial (use Serial1 or Serial2 if available!)
const int pins = 6;  // number of channels for servos
Servo servos[pins];  // allocate servos for all channels
int pwmPins[] = {2, 0, 4, 5, 6, 7};  // servo pins 
int failSaveValues[] = {0,90,90,90,90,90}; // neutrol positions


void setup() {
  SpektrumSerial.begin(9600);

  // Log
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  // Activate the loggin to the console only if SpektrumSatellite is not using Serial
  satellite.setLog(Serial);

  //scale the values from 0 to 180 degrees for PWM
  satellite.setChannelValueRange(0, 180);

  
  // setup PWM pins
  for (int j=0;j<pins; j++){
    servos[j].attach(pwmPins[j]);
  }

  // we use the built in LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN,LOW);

  // wait forever for data
  satellite.waitForData();

  Serial.println("setup - DONE!");
}

void loop() {
  
  if (satellite.getFrame()) {   
    for (int j=0;j<pins; j++){
       Channel ch = static_cast<Channel>(j);   
       long value = satellite.getChannelValue(ch);
       servos[j].write(value);
    }        
    digitalWrite(LED_BUILTIN,HIGH);
  } 

  // if we loose the connection we set the values to neutral 
  if (!satellite.isConnected()) {
    satellite.log("Invoking fail save values");   
    for (int j=0;j<pins; j++){
       servos[j].write(failSaveValues[j]);
    }        
    digitalWrite(LED_BUILTIN,LOW);
  }
  
}

```



