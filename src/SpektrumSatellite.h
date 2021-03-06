/**
 * Arduino API for the Spectrum Satellite Receiver. We support Arduinos, ESP32, ESP8266 etc.
 * - Complete Implementation of Spec (See https://www.spektrumrc.com/ProdInfo/Files/Remote%20Receiver%20Interfacing%20Rev%20A.pdf)
 * - Optional Support of logging using a specified Serial pin
 * - Support of Automatic scaling of channel values (using different types)
 * - Support for all channels
 * - Support for binding using different BindModes
 * - Automatic handling of 1024 or 2048 servo data
 * - Support for sending data
 * - Checks the endianness (in Arudino the processor is little endian, the protocal sends all data fields as big-endian)
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
#define SEND_BUFFER_SIZE sizeof(Data)
#define BINDING_PULSE_DELAY_MS 100
#define SPEKTRUM_SATELLITE_BPS 125000


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

// Define all 12 Available Channels 
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

// Header of a frame
union Header {
  uint16_t fades;
  struct __attribute__((__packed__)) Internal {
    byte fades;
    byte system;
  } internal;
};

struct __attribute__((__packed__)) Data {
  Header header;
  uint16_t values[7];
};


#include "Arduino.h"
#include "Scaler.h"


template <class T>
class SpektrumSatellite {
  public:
    // Constructor
    SpektrumSatellite(Stream &serial); 
    
    // Defines the binding mode  
    void setBindingMode(BindMode bindMode); 
    
    // Set the receiver in binding mode 
    void startBinding(unsigned powerPin, unsigned rxPin);
    
    // Receive a data record from the Satellite Receiver
    bool getFrame(int timeout=DEFAULT_RECEIVING_TIMEOUT);

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

    void sendData();
    void sendData(uint8_t* str);
    
    // Checks that we did not time out
    bool isConnected();
    bool isConnected(long timeoutMs);
    
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

    // checks if the system is valid
    boolean isValidSystem(int system);
 
    // checks if binding is in internal mode
    boolean isInternal();

    // switch endianness if necessary for processors which are little endian
    void switchEndianness();

    // checks if the biggest number is 2048 (instead of 1024)
    bool is2048();
    
    // Determines the fades
    uint16_t getFades();

    Status getStatus();

    // Defines that we need to process all data (use on reliable connectins only)
    void setProcessAllData(bool flag);
    
    // == usually not needed but in case when you need to access the data
    bool parseFrame(byte* inData);
    bool parseFrame(Data* inData);
    Data* getSendBuffer(boolean auxData);
    Data* getSendBuffer();
    // logging
    void setLog(Stream& log);
    void setLogMod(long value);
    void log(const char*);
    void log1(const char*);
    void log(const char*, const char*);
    void log(const char*, int value);
    void logHex(const char*, int value);
    // provides the unconverted channel values
    uint16_t* getChannelValuesRaw();
  

  private:
    uint16_t channelValues[12];
    Data dataPacket; //;uint16_t sendValues[7];
    unsigned long timeOfLastRead;
    unsigned long successCount;
    unsigned long failCount;
    unsigned long frameCount;
    unsigned long sendCount;
    uint16_t maskCHANID;
    uint16_t maskVALUE;
    uint16_t fades;
    System system;
    boolean isInternalFlag;
    boolean isSendAuxData;
    boolean isSwapBytes;
    boolean processAllData = false;
    
    Stream *serial;
    Stream *serialLog = NULL;
    Scaler<T> scaler;
    BindMode bindMode; 
    Status status;   
    long logMod = 1000;
    
    // private methods
    void logFrame(long available, bool result);
    void swapBytes(uint16_t* value);
};

// 12 channels
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
SpektrumSatellite<T>::SpektrumSatellite(Stream& serial) {
    // Internal_DSMx_11ms is recommended bind value
    this->serial = &serial;
    setBindingMode(Internal_DSMx_11ms);

    // setup Initial Status
    this->status = NotConnected;

    // check endianness
    int n = 1;
    if(*(char *)&n == 1) {
      // little endian if true
       this->isSwapBytes = true;
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

  // update the system information
  if (bindMode == Internal_DSM2_22ms || bindMode == External_DSM2_22ms){
    setSystem(DSM2_22MS_1024);
  } else if (bindMode == Internal_DSM2_11ms || bindMode == External_DSM2_11ms){
    setSystem(DSM2_11MS_2048);
  } else if (bindMode == Internal_DSMx_11ms || bindMode == External_DSMx_11ms) {
    setSystem(DSMX_11MS_2048);
  } else if (bindMode == Internal_DSMx_22ms || bindMode == External_DSMx_22ms) {
    setSystem(DSMS_22MS_2048);
  }    
  
  // update the internal flag
  if (bindMode == Internal_DSM2_11ms || bindMode == Internal_DSM2_22ms || bindMode == Internal_DSMx_11ms || bindMode == Internal_DSMx_22ms ){
    isInternalFlag = true;
  } else {
    isInternalFlag = false;
  }

  log("-> isInternal:", isInternal() ? "true":"false");
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

  if (system == DSM2_22MS_1024){
    maskCHANID = MASK_1024_CHANID;
    maskVALUE = MASK_1024_SXPOS;
  } else {
    maskCHANID = MASK_2048_CHANID;
    maskVALUE = MASK_2048_SXPOS;
  }

}

template <class T>
boolean SpektrumSatellite<T>::isInternal() {
  return this->isInternalFlag;
}

template <class T>
boolean SpektrumSatellite<T>::isValidSystem(int system) {
  bool result = false;
  if (isInternal()) {
    // check system
    if (system == DSM2_22MS_1024
     || system == DSM2_11MS_2048
     || system == DSMS_22MS_2048
     || system == DSMX_11MS_2048 ){
        result = true;
     }
    if (!result) {
      log("isValidSystem: ",result ? "true":"false");
      logHex("system: ",system);
    }
  } else {
    // system has no meaning
    result = true;
  }
  return result;  
}

template <class T>
void SpektrumSatellite<T>::setProcessAllData(bool flag){
  processAllData = flag;
}

template <class T>
bool SpektrumSatellite<T>::parseFrame(byte* inData){
  return parseFrame((Data*) inData);
}

template <class T>
bool SpektrumSatellite<T>::parseFrame(Data* inData){
    Data* data =  (Data*) inData;
    static bool systemReported = false;
    // a frame is 16 bytes -> 7 channels + fades
    // determine system and fades
    if (isInternal()){
        this->fades = data->header.internal.fades;
        if (!systemReported){
          System recevedSystem = (System) data->header.internal.system;
          logHex("System from the Satellite:", recevedSystem);
          systemReported = true;
          if (recevedSystem != getSystem()){
            if (isValidSystem(recevedSystem))
              setSystem(recevedSystem);
            else
              logHex("Unexpected system", recevedSystem);
          }
        }
    } else {
        this->fades = data->header.fades;
    }

    // determine channel values
    uint16_t channelShift = is2048() ? 11 : 10;  
    for (int i = 0; i < 7; i++) {
      uint16_t inValue = data->values[i];
      swapBytes(&inValue);
      uint16_t channelID = (inValue & maskCHANID)>>channelShift;
      uint16_t channelValue = inValue & maskVALUE;   

      //Serial.print(inValue);
      //Serial.print(" / ");
      //Serial.print(channelID);
      //Serial.print("=>");
      //Serial.println(channelValue);

      if (channelID>=0 && channelID<MAX_CHANNELS) {
        channelValues[channelID] = channelValue; 
      } else {
        //log("Invalid Channel in parseFrame: ",channelID);
      }
    }
    return true;
}

template <class T>
void SpektrumSatellite<T>::swapBytes(uint16_t* value){
  if (isSwapBytes) {
    *value= ((*value<<8)&0xff00)|((*value>>8)&0x00ff);  
  }
}

template <class T>
void SpektrumSatellite<T>::switchEndianness() {
   this->isSwapBytes = !this->isSwapBytes;
}

template <class T>
bool SpektrumSatellite<T>::getFrame(int transactionTimeMs){
    short inByte;
    byte inData[SEND_BUFFER_SIZE];
    bool result = false;
    long available = serial->available();

    //  16-byte data packet every 11ms or 22ms, 
    if (available >= 16) {
      timeOfLastRead = millis();
      // resychronize and use last data
      if (!processAllData && available > 16){
        long diff = available - 16;
        log("skipping number of bytes:", diff);
        // skip unnecessary data
        for (int j=0;j<diff;j++)
          serial->read();
      }
      
      // read the latest data packet
      inByte = serial->readBytes(inData, 16);
      if (inByte!=16){
        log("We could not read all data");
        result = false;
      } else {
        // check if we processed the data within the indicated time period
        result = isConnected(transactionTimeMs);
        if (result){
          parseFrame(inData);
          // check if the frame is valid
          result = isValidSystem(this->system);
          status = Receiving;

          // log the status
          logFrame(available, result);    
        } else {
          log("Frame ignored because of timeout");
        }
      }
    }
    
    return result;
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
  if (channelId>=Throttle && channelId<= Aux7){
    channelValues[channelId] = scaler.deScale(value);
    if (channelId>=Aux1){
      isSendAuxData = true;
    } else {
      log("Invalid Channel Number:", channelId);
    }
  }
}

template <class T>
T SpektrumSatellite<T>::getChannelValue(Channel channelId){
  return scaler.scale(channelValues[channelId]);
}

template <class T>
const char* SpektrumSatellite<T>::getChannelName(Channel channelId){
  return ChannelNames[channelId];
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
  setChannelValue(Aux2,value);
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
Data *SpektrumSatellite<T>::getSendBuffer() {
  return getSendBuffer(false);
}

template <class T>
Data *SpektrumSatellite<T>::getSendBuffer(boolean auxData) {
    memset(&dataPacket,0,sizeof(Data));

    // determine the position of the index info
    uint16_t channelShift = is2048() ? 11 : 10;  
    // update the data in the sendValues buffer
    if (auxData){
      for(int j=6;j<MAX_CHANNELS;j++){
        dataPacket.values[j-6+1] = channelValues[j] & maskVALUE | j<<channelShift;
        swapBytes(&dataPacket.values[j-6+1]);
      }
    } else {
      for(int j=0;j<7;j++){
        dataPacket.values[j+1] = channelValues[j] & maskVALUE | j<<channelShift;
        swapBytes(&dataPacket.values[j+1]);
      }
    }

    // if the mode is internal we need to add the system id
    Header* header = &(dataPacket.header);
    if (isInternal()) {
      header->internal.fades = this->fades;
      header->internal.system = this->system;
    } else {
      header->fades = this->fades;
    }

    return &dataPacket;
}


template <class T>
void SpektrumSatellite<T>::sendData(){
  if (sendCount>0 && sendCount++%logMod==0){
    log("sendData");
  }
  Data* data =  getSendBuffer();
  serial->write((byte*)data,SEND_BUFFER_SIZE);

  // send Aux if necessary
  if (isSendAuxData){ 
    Data* data =  getSendBuffer(true);
    serial->write((byte*)data,SEND_BUFFER_SIZE);
  }
}

template <class T>
void SpektrumSatellite<T>::sendData(uint8_t* str){
  if (logMod>0 && sendCount++%logMod==0){
    log((char*)str);
  }
  serial->print((char*)str);
  serial->flush();
}


template <class T>
bool SpektrumSatellite<T>::isConnected(){
  return isConnected(TRANSACTION_TIME);
}

template <class T>
bool SpektrumSatellite<T>::isConnected(long transactionTime){
  return  (millis() - timeOfLastRead < transactionTime);
}


template <class T>
void SpektrumSatellite<T>::waitForData(){
  log("waitForData");
  while(!serial->available()) {
      log1(".");
      delay(1000);
  }
}

template <class T>
void SpektrumSatellite<T>::setChannelValueRange(T min, T max){
  log("setChannelValueRange");
  // set ouput value range
  scaler.setActive(true);
  T inMax = is2048() ? 2048: 1024;
  scaler.setValues(0,inMax,min,max);
  log("setChannelValueRange <-");
}

template <class T>
bool SpektrumSatellite<T>::is2048() {
  return this->system==DSM2_22MS_1024?false:true;
}

template <class T>
uint16_t SpektrumSatellite<T>::getFades() {
  return this->fades;
}

template <class T>
Scaler<T>* SpektrumSatellite<T>::getScaler(){
  return &this.scaler;
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
void SpektrumSatellite<T>::log1(const char* str) {
  if (serialLog==NULL) return;
  serialLog->print(" ");
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
void SpektrumSatellite<T>::setLogMod(long value) {
  this->logMod = value;
}


template <class T>
void SpektrumSatellite<T>::logFrame(long available, bool result) {
    if (result) {
      successCount++;
    } else {
      failCount++;
    }
    if (logMod>0) {
      if (getStatus()==Receiving){
        if (frameCount%logMod==0){
          log("getFrame");
          log("available data:",available);
          log("-> isConnected:",isConnected()?"true":"false");
          log("-> isValidSystem:",isValidSystem(this->system)?"true":"false");
          log("-> frameCount:",frameCount);
          log("-> successCount:",successCount);
          log("-> failCount:",failCount);
        } 
      } else {
        if (serialLog!=NULL) {
          serialLog->print(available>0 ? "+" : ".");
        }
      }
    }
    frameCount++;
}


template <class T>
void SpektrumSatellite<T>::setLog(Stream& logSer){
  this->serialLog = &logSer;
}

template <class T>
uint16_t* SpektrumSatellite<T>::getChannelValuesRaw() {
  return channelValues;
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
    
      pinMode(rxPin, OUTPUT);    // sets the digital pin as output
      digitalWrite(rxPin, LOW);  // make sure that the pin off
      pinMode(powerPin, OUTPUT);    // sets the digital pin as output
      digitalWrite(powerPin, LOW);  // make sure that the pin off
      digitalWrite(rxPin, HIGH); // set initial state to high
      delay(2000);

      // To put a receiver into bind mode, within 200ms of power application the host device 
      // needs to issue a series of falling pulses
      pinMode(powerPin, OUTPUT);    // sets the digital pin as output
      digitalWrite(powerPin, HIGH);  // make sure that the pin off
      delay(50);
      
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
    }
}


#endif /* SPECTRUMSATELLITE_H_ */
