/**
 * Arduino API for the Spectrum Satellite Receiver. We support Arduinos, ESP32, ESP8266 etc.
 * - Complete Implementation of Spec (See https://www.spektrumrc.com/ProdInfo/Files/Remote%20Receiver%20Interfacing%20Rev%20A.pdf)
 * - Optional Support of logging using a specified Serial pin
 * - Support of Automatic scaling of channel values (using different types)
 * - Support for all channels
 * - Support for binding using different BindModes
 * - Automatic handling of 1024 or 2048 servo data
 * - Support for sending data
 */

#ifndef SPECTRUMSATELLITE_H_
#define SPECTRUMSATELLITE_H_

#define TRANSACTION_TIME 1000
#define DEFAULT_RECEIVING_TIMEOUT 10000
#define MAX_CHANNELS 12
#define MASK_1024_CHANID 0xFC00
#define MASK_1024_SXPOS 0x03FF
#define MASK_2048_CHANID 0x7800
#define MASK_2048_SXPOS 0x07FF 
#define SEND_BUFFER_SIZE 16
#define BINDING_PULSE_DELAY_MS 100


// Defines the number of falling pulses when Binding
enum BindMode {
  Internal_DSM2_22ms=3,
  External_DSM2_22ms=4, 
  Internal_DSM2_11ms=5, 
  External_DSM2_11ms=6, 
  Internal_DSMx_22ms=7, 
  External_DSMx_22ms=8, 
  Internal_DSMx_11ms=9, 
  External_DSMx_11ms=10  
};

// Is it working ?
enum Status {
  NotConnected,
  Binding,
  Receiving
};

// Define all Available Channels 
enum Channel {
  Throttle,
  Aileron,
  Elevator,
  Rudder,
  Gear,
  Aux1,
  Aux2,
  Aux3,
  Aux4,
  Aux5,
  Aux6,
  Aux7
};

// Supported Systems
enum System {
   DSM2_22MS_1024=0x01,
   DSM2_11MS_2048=0x12,
   DSMS_22MS_2048=0xa2,
   DSMX_11MS_2048=0xb2  
};

#include "Arduino.h"
#include "Scaler.h"
#include "SpektrumCSV.h"



template <class T>
class SpektrumSatellite {
  public:
    // Constructor
    SpektrumSatellite(HardwareSerial &serial); 
    
    // Defines the binding mode  
    void setBindingMode(BindMode bindMode); 
    
    // Set the receiver in binding mode 
    void startBinding(unsigned powerPin, unsigned rxPin);
    
    // Receive a data record from the Satellite Receiver
    bool getFrame();
    
    // Gets the scaled value for the indicated channel
    T getChannelValue(Channel channelId);
    T getThrottle();
    T getAileron();
    T getElevator();
    T getRudder();
    T getGear();
    T getAux1();
    T getAux2();
    T getAux3();
    T getAux4();
    T getAux5();
    T getAux6();
    T getAux7();

    void setChannelValue(Channel channelId, T value);
    void setThrottle(T value);
    void setAileron(T value);
    void setElevator(T value);
    void setRudder(T value);
    void setGear(T value);
    void setAux1(T value);
    void setAux2(T value);
    void setAux3(T value);
    void setAux4(T value);
    void setAux5(T value);
    void setAux6(T value);
    void setAux7(T value);

    uint16_t* getSendBuffer(boolean auxData);
    uint16_t* getSendBuffer();
    void sendData(Stream& out);
    
    // Checks that we did not time out
    bool isTransaction();
    
    // Test if we are receiving any valid data within the indicated time
    bool isReceivingData(unsigned long timeOut);
    
    // Test if we are receiving any valid data within the default time
    bool isReceivingData();

    // wait for any data
    void waitForData();
    
    // Provides the channel name as string
    const char* getChannelName(Channel channelId);
    
    // Defines the maximum value that we expect or provide to the API  
    void setChannelValueRange(T min, T max);
    
    // Provides access to the Scaler
    Scaler<T> *getScaler();
    
    // Sets the system which defines the data format (e.g. 1024 or 2048 servo data)
    void setSystem(System system);
    
    // Determines the system
    System getSystem();
    bool is2048();
    
    // Determines the fades
    uint16_t getFades();

    Status getStatus();
    
    // Activate/Deactivates logging
    void setLog(Stream& log);
    // write info to the log
    void log(const char*);
    void log(const char*, const char*);
    void log(const char*, int value);
    void logHex(const char*, int value);
 
  private:
    uint16_t channelValues[12];
    uint16_t sendValues[8];
    unsigned long time;
    unsigned long successCount;
    unsigned long failCount;
    unsigned long frameCount;
    uint16_t maskCHANID;
    uint16_t maskVALUE;
    uint16_t fades;
    System system;
    boolean isInternal;
    boolean isSendAuxData;
    
    HardwareSerial *serial;
    Stream *serialLog;
    Scaler<T> *scaler;
    Scaler<T> *sendScaler;
    BindMode bindMode; 
    Status status;   
    
    // private methods
    void setupSerial();
    boolean isValidSystem();
    void logFrame(boolean result);
};


const static char* ChannelNames[] = {
  "Throttle",
  "Aileron",
  "Elevator",
  "Rudder",
  "Gear",
  "Aux1",
  "Aux2",
  "Aux3",
  "Aux4",
  "Aux5",
  "Aux6",
  "Aux7"
};

template <class T>
SpektrumSatellite<T>::SpektrumSatellite(HardwareSerial& serial) {
    // Internal_DSMx_11ms is recommended bind value
    setBindingMode(Internal_DSMx_11ms);
    this->serial = &serial;
    if (!serial) {
      setupSerial();
    }

    // setup Initial Status
    this->status = NotConnected;
    if (isReceivingData(1000)){
      this->status = Receiving;    
    }
}

template <class T>
Status SpektrumSatellite<T>::getStatus(){
  return this->status;
}

template <class T>
void SpektrumSatellite<T>::setBindingMode(BindMode bindMode) {
  log("setBindingMode");
  this->bindMode = bindMode;
  if (bindMode == Internal_DSM2_22ms || bindMode == External_DSM2_22ms){
    maskCHANID = MASK_1024_CHANID;
    maskVALUE = MASK_1024_SXPOS;
    system = DSM2_22MS_1024;
  } else {
    maskCHANID = MASK_2048_CHANID;
    maskVALUE = MASK_2048_SXPOS;

    if ( Internal_DSM2_11ms || bindMode == External_DSM2_11ms){
      system = DSM2_11MS_2048;
    } else if (bindMode == Internal_DSMx_11ms || bindMode == External_DSMx_11ms) {
      system = DSMX_11MS_2048;
    } else if (bindMode == Internal_DSMx_22ms || bindMode == External_DSMx_22ms) {
      system = DSMS_22MS_2048;
    }    
  }

  if (bindMode == Internal_DSM2_11ms || bindMode == Internal_DSM2_22ms || bindMode == Internal_DSMx_11ms || bindMode == Internal_DSMx_22ms ){
    isInternal = true;
  } else {
    isInternal = false;
  }

  log("-> isInternal:", isInternal ? "true":"false");
  logHex("-> system:", system);
  
}

template <class T>
System SpektrumSatellite<T>::getSystem(){
  return this->system;
}

template <class T>
void SpektrumSatellite<T>::setSystem(System system){
  logHex("setSystem:", system);
  this->system = system;

}

template <class T>
boolean SpektrumSatellite<T>::isValidSystem() {
  boolean result = system == 0x01 || system == 0x12 || system == 0xa2 || system == 0xb2; 
  if (!result)
    log("isValidSystem: ",result ? "true":"false");
  return result;  
}


/**
 * The Spektrum Satellite power pin i using 3.3VDC +/-5%, 20mA max. However for the ESP8266 The 
 * maximum current that can be drawn from a single GPIO pin is 12mA.
 */
template <class T>
void SpektrumSatellite<T>::startBinding(unsigned powerPin, unsigned rxPin) {  
    // switch off serial interface
    if(serial) {
      log("startBinding");

      serial->end();
      delay(500);
    
      pinMode(rxPin, OUTPUT);    // sets the digital pin as output
      digitalWrite(rxPin, LOW);  // make sure that the pin off
      delay(1000);

      // To put a receiver into bind mode, within 200ms of power application the host device 
      // needs to issue a series of falling pulses
      pinMode(powerPin, OUTPUT);    // sets the digital pin as output
      digitalWrite(rxPin, HIGH); // sets the digital pin on
      digitalWrite(powerPin, HIGH);  // make sure that the pin off
      
      log("-> number of pulses: ", bindMode);
      //noInterrupts();

      for (int j=0; j<bindMode;j++) {
        digitalWrite(rxPin, HIGH); // sets the digital pin on
        delayMicroseconds(BINDING_PULSE_DELAY_MS);            // waits 
        digitalWrite(rxPin, LOW);  // sets the digital pin  off
        delayMicroseconds(BINDING_PULSE_DELAY_MS);            // waits 
      }
      //digitalWrite(rxPin, HIGH); // sets the digital pin on
      //interrupts();
      log("-> number of pulses DONE");

      delay(500);      
      pinMode(rxPin, INPUT);    // sets the digital pin 13 as input
      // we need to re-activate the serial interface again
      delay(500);
      setupSerial();
      delay(500);      
    }
}

template <class T>
void SpektrumSatellite<T>::setupSerial(void) {
  //  125000bps, 8 bits, No parity, 1 stop (8N1). 
  if (serial) {
     log("setupSerial");
     serial->begin(125000, SERIAL_8N1);
  }
}

template <class T>
bool SpektrumSatellite<T>::getFrame(){
    short inByte;
    byte inData[16];
    boolean result = false;
    log("available data:",serial->available());
    
    //  16-byte data packet every 11ms or 22ms, 
    if (serial->available() >= 16) {
      time = millis();
      inByte = serial->readBytes(inData,16);

      // determine system and fades
      if (isInternal){
         system = (System)inData[1];
         fades = inData[0];
      } else {
         fades = inData[0] * 256 + inData[1];
      }

      // determine channel values
      for (int i = 1; i < MAX_CHANNELS; i++) {
        short inValue = inData[i * 2] * 256 + inData[i * 2 + 1];
        short channelID = inValue & maskCHANID;
        if (channelID>=0 && channelID<MAX_CHANNELS) {
          short channelValue = inValue & maskVALUE;
          channelValues[channelID] = channelValue; 
        }
      }
    
      // check if the frame is valid
      result = isInternal ? isValidSystem() && isTransaction() : isTransaction();
      // update the status
      if (result){
        status = Receiving;
      }
    } 

    // log the status
    logFrame(result);    
    
    return result;
}

template <class T>
void SpektrumSatellite<T>::logFrame(boolean result) {
    if (result) {
      successCount++;
    } else {
      failCount++;
    }
    long mod = getStatus()==Receiving ? 1000 : 1;
    if (frameCount%mod==0){
      log("getFrame");
      log("-> isTransaction:",isTransaction()?"true":"false");
      log("-> isValidSystem:",isValidSystem()?"true":"false");
      log("-> frameCount:",frameCount);
      log("-> successCount:",successCount);
      log("-> failCount:",failCount);
    }
    frameCount++;
}

template <class T>
T SpektrumSatellite<T>::getChannelValue(Channel channelId){
  return scaler->scale(channelValues[channelId]);
}

template <class T>
const char* SpektrumSatellite<T>::getChannelName(Channel channelId){
  return ChannelNames[channelId];
}

template <class T>
T SpektrumSatellite<T>::getThrottle(){
    return getChannelValue(Throttle);
}

template <class T>
T SpektrumSatellite<T>::getAileron(){
  return getChannelValue(Aileron);
}

template <class T>
T SpektrumSatellite<T>::getElevator(){
  return getChannelValue(Elevator);
}

template <class T>
T SpektrumSatellite<T>::getRudder(){
  return getChannelValue(Rudder);
}

template <class T>
T SpektrumSatellite<T>::getGear(){
  return getChannelValue(Gear);
}

template <class T>
T SpektrumSatellite<T>::getAux1(){
  return getChannelValue(Aux1);
}

template <class T>
T SpektrumSatellite<T>::getAux2(){
  return getChannelValue(Aux2);
}

template <class T>
T SpektrumSatellite<T>::getAux3(){
  return getChannelValue(Aux3);
}

template <class T>
T SpektrumSatellite<T>::getAux4(){
  return getChannelValue(Aux4);
}

template <class T>
T SpektrumSatellite<T>::getAux5(){
  return getChannelValue(Aux5);
}

template <class T>
T SpektrumSatellite<T>::getAux6(){
  return getChannelValue(Aux6);
}

template <class T>
T SpektrumSatellite<T>::getAux7(){
  return getChannelValue(Aux7);
}

template <class T>
void SpektrumSatellite<T>::setChannelValue(Channel channelId, T value) {
  if (channelId>=0 && channelId<= Aux7){
    channelValues[channelId] = scaler->deScale(value);
    if (channelId>=Aux1){
      isSendAuxData = true;
    }
  }
}

template <class T>
void SpektrumSatellite<T>::setThrottle(T value) {
  setChannelValue(Throttle,value);
}

template <class T>
void SpektrumSatellite<T>::setAileron(T value){
  setChannelValue(Aileron,value);
}

template <class T>
void SpektrumSatellite<T>::setElevator(T value){
  setChannelValue(Elevator,value);
}

template <class T>
void SpektrumSatellite<T>::setRudder(T value){
  setChannelValue(Rudder,value);
}

template <class T>
void SpektrumSatellite<T>::setGear(T value){
  setChannelValue(Gear,value);
}

template <class T>
void SpektrumSatellite<T>::setAux1(T value){
  setChannelValue(Aux1,value);
}

template <class T>
void SpektrumSatellite<T>::setAux2(T value){
  setChannelValue(Aux3,value);
}

template <class T>
void SpektrumSatellite<T>::setAux3(T value){
  setChannelValue(Aux3,value);
}

template <class T>
void SpektrumSatellite<T>::setAux4(T value){
  setChannelValue(Aux4,value);
}

template <class T>
void SpektrumSatellite<T>::setAux5(T value){
  setChannelValue(Aux5,value);
}

template <class T>
void SpektrumSatellite<T>::setAux6(T value){
  setChannelValue(Aux6,value);
}

template <class T>
void SpektrumSatellite<T>::setAux7(T value){
  setChannelValue(Aux7,value);
}

template <class T>
uint16_t *SpektrumSatellite<T>::getSendBuffer() {
  return getSendBuffer(false);
}

template <class T>
uint16_t *SpektrumSatellite<T>::getSendBuffer(boolean auxData) {

    // if the mode is internal we need to add the system id
    if (isInternal) {
      // limit number of fades to maximum value to prevent conflicts
      // with the system info
      if (this->fades>0xFF){
        fades = 0xFF;
      }
      sendValues[0] = this->fades << 8 | system;      
    }{
      sendValues[0] = this->fades;
    }

    // determine the position of the index info
    uint16_t channelShift = is2048() ? 11 : 10;  
    // update the data in the sendValues buffer
    if (auxData){
      for(int j=7;j<Aux7;j++){
        sendValues[j-7+1] = channelValues[j-7] && maskVALUE | j<<channelShift;
      }
    } else {
      for(int j=0;j<7;j++){
        sendValues[j+1] = channelValues[j] && maskVALUE | j<<channelShift;
      }
    }
    return sendValues;
}


template <class T>
void SpektrumSatellite<T>::sendData(Stream& out){
  uint16_t* data = getSendBuffer();
  for (int j=0;j<8;j++){
    out.write(data[j]);
  }

  // send Aux if necessary
  if (isSendAuxData){ 
    for (int j=0;j<8;j++){
      out.write(data[j]);
    }
  }
}

template <class T>
bool SpektrumSatellite<T>::isTransaction(){
  return (millis() - time < TRANSACTION_TIME);
}

template <class T>
void SpektrumSatellite<T>::waitForData(){
  log("waitForData");
  boolean validData = false;
  while(!validData) {
      validData = getFrame();
      delay(1000);
  }
}

template <class T>
bool SpektrumSatellite<T>::isReceivingData(unsigned long timeOut) {
  bool result = false;
  unsigned long start = millis();
  while(!result) {
      result = getFrame();
      if (millis() - start > timeOut){
        break;
      }
      delay(1000);
  }
  log("isReceivingData:",result?"true":"false");
  return result;
}

template <class T>
bool SpektrumSatellite<T>::isReceivingData(){
  return isReceivingData(DEFAULT_RECEIVING_TIMEOUT);  // 10 sec
}

template <class T>
void SpektrumSatellite<T>::setChannelValueRange(T min, T max){
  // set ouput value range
  scaler->setActive(true);
  scaler->setValues(0,is2048() ? 2048: 1024,min,max);
}

template <class T>
bool SpektrumSatellite<T>::is2048() {
  return this->system==0x01?false:true;
}

template <class T>
uint16_t SpektrumSatellite<T>::getFades() {
  return this->fades;
}

template <class T>
Scaler<T>* SpektrumSatellite<T>::getScaler(){
  return this->scaler;
}

template <class T>
void SpektrumSatellite<T>::log(const char* str) {
  if (serialLog==NULL) return;
  serialLog->println(str);
}

template <class T>
void SpektrumSatellite<T>::log(const char* str, const char* str1) {
  if (serialLog==NULL) return;
  serialLog->print(str);
  serialLog->print(" ");
  serialLog->println(str1);
}

template <class T>
void SpektrumSatellite<T>::log(const char* str, int value) {
  if (serialLog==NULL) return;
  serialLog->print(str);
  serialLog->print(" ");
  serialLog->println(value);
}

template <class T>
void SpektrumSatellite<T>::logHex(const char* str, int value) {
  if (serialLog==NULL) return;
  serialLog->print(str);
  serialLog->print(" ");
  serialLog->println(value,HEX);
}

template <class T>
void SpektrumSatellite<T>::setLog(Stream& logSer){
  this->serialLog = &logSer;
}


#endif /* SPECTRUMSATELLITE_H_ */
