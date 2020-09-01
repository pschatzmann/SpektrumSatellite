# Spectrum Satellite API for Arduino

 Arduino API for the Spectrum Satellite Receiver. We support Arduinos, ESP32, ESP8266 etc.

 - Complete Implementation of the Specification (See https://www.spektrumrc.com/ProdInfo/Files/Remote%20Receiver%20Interfacing%20Rev%20A.pdf)
 - Support for sending data
 - Support for receiving data
 - Support for all channels
 - Support for binding using different BindModes
 - Automatic handling of 1024 or 2048 servo data
 - Support of different data types and automatic scaling of channel values 
 - Optional Support of logging using a specified Serial pin

## Usage Scenarios
The following usage scenarios are supported and documented with examples
 - Binding of a Spektrum Satellite (Bind)
 - Implement a Receiver:  Receive Serial Satellite Data and update PWN Pins (Receive)
 - Implement a Remote Control Radio: Reads Analog input Pins and sends it out in the Serial Spektrum Format  - Receive Serial Satellite Data and send it e.g. via UDP (Gateway)
 - Receive Serial Satellite Data and send it as CSV e.g. via UDP (GatewayCSV)
(SendUDP)



