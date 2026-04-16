/**
 * @file SpektrumSatellite.h
 * @brief Arduino API for Spektrum Satellite receivers.
 *
 * We support Arduinos, ESP32,
 * ESP8266 etc.
 * - Complete Implementation of Spec (See
 * https://www.spektrumrc.com/ProdInfo/Files/Remote%20Receiver%20Interfacing%20Rev%20A.pdf)
 * - Optional Support of logging using a specified Serial pin
 * - Support of Automatic scaling of channel values (using different types)
 * - Support for all channels
 * - Support for binding using different BindModes
 * - Automatic handling of 1024 or 2048 servo data
 * - Support for sending data
 * - Checks the endianness (in Arudino the processor is little endian, the
 * protocal sends all data fields as big-endian)
 * @author Phil Schatzmann
 */

#pragma once

#include "Arduino.h"  // millis, Stream
#include "Scaler.h"

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

namespace spektrum_satellite {

/**
 * @enum BindMode
 * @brief Number of bind pulses sent after power-up.
 */
enum BindMode {
  Internal_DSM2_22ms = 3,  ///< Internal DSM2 at 22ms.
  External_DSM2_22ms = 4,  ///< External DSM2 at 22ms.
  Internal_DSM2_11ms = 5,  ///< Internal DSM2 at 11ms.
  External_DSM2_11ms = 6,  ///< External DSM2 at 11ms.
  Internal_DSMx_22ms = 7,  ///< Internal DSMX at 22ms.
  External_DSMx_22ms = 8,  ///< External DSMX at 22ms.
  Internal_DSMx_11ms = 9,  ///< Internal DSMX at 11ms.
  External_DSMx_11ms = 10  ///< External DSMX at 11ms.
};

/**
 * @enum Status
 * @brief Runtime receiver state.
 */
enum Status {
  NotConnected,  ///< No recent valid frame.
  Binding,       ///< Binding procedure in progress.
  Receiving      ///< Frames are being received.
};

/**
 * @enum Channel
 * @brief Supported channel indices.
 */
enum Channel {
  Throttle,  ///< Throttle channel.
  Aileron,   ///< Aileron channel.
  Elevator,  ///< Elevator channel.
  Rudder,    ///< Rudder channel.
  Gear,      ///< Gear channel.
  Aux1,      ///< Auxiliary channel 1.
  Aux2,      ///< Auxiliary channel 2.
  Aux3,      ///< Auxiliary channel 3.
  Aux4,      ///< Auxiliary channel 4.
  Aux5,      ///< Auxiliary channel 5.
  Aux6,      ///< Auxiliary channel 6.
  Aux7       ///< Auxiliary channel 7.
};

/**
 * @enum System
 * @brief Spektrum protocol system identifiers.
 */
enum System {
  DSM2_22MS_1024 = 0x01,  ///< DSM2, 22ms, 1024 mode.
  DSM2_11MS_2048 = 0x12,  ///< DSM2, 11ms, 2048 mode.
  DSMS_22MS_2048 = 0xa2,  ///< DSMX/DSMS, 22ms, 2048 mode.
  DSMX_11MS_2048 = 0xb2   ///< DSMX, 11ms, 2048 mode.
};

/// Header of a frame
union Header {
  uint16_t fades;
  struct __attribute__((__packed__)) Internal {
    byte fades;
    byte system;
  } internal;
};

/// Data packet structure for parsing and sending frames
struct __attribute__((__packed__)) Data {
  Header header;
  uint16_t values[7];
};

/// 12 channels as strings
const static char* ChannelNames[] = {
    "Throttle", "Aileron", "Elevator", "Rudder", "Gear", "Aux1",
    "Aux2",     "Aux3",    "Aux4",     "Aux5",   "Aux6", "Aux7"};

/**
 * @brief Spektrum Satellite protocol API.
 *
 * Provides bind support, frame receive/transmit, channel access, optional
 * scaling, protocol mode handling (1024/2048), and optional logging.
 *
 * @tparam T Public channel value type (e.g. uint16_t, int, float).
 * @tparam ScalerT Optional custom scaler class (default is Scaler<T>).
 * @author Phil Schatzmann
 */
template <class T = uint16_t, class ScalerT = Scaler<T>>
class SpektrumSatellite {
 public:
  /// @brief Constructs the API using the given transport stream.
  SpektrumSatellite(Stream& serial) {
    // Internal_DSMx_11ms is recommended bind value
    this->serial = &serial;
    this->rangeMin = 0;
    this->rangeMax = 0;
    this->isSwapBytes = false;
    this->isSystemReported = false;
    this->isChannelRangeConfigured = false;
    setBindingMode(Internal_DSMx_11ms);

    // setup Initial Status
    this->status = NotConnected;

    // check endianness
    int n = 1;
    if (*(char*)&n == 1) {
      // little endian if true
      this->isSwapBytes = true;
    }
  }

  /// @brief Sets the receiver binding mode.
  void setBindingMode(BindMode bindMode) {
    log("setBindingMode");
    this->bindMode = bindMode;
    this->isSystemReported = false;

    // update the system information
    if (bindMode == Internal_DSM2_22ms || bindMode == External_DSM2_22ms) {
      setSystem(DSM2_22MS_1024);
    } else if (bindMode == Internal_DSM2_11ms ||
               bindMode == External_DSM2_11ms) {
      setSystem(DSM2_11MS_2048);
    } else if (bindMode == Internal_DSMx_11ms ||
               bindMode == External_DSMx_11ms) {
      setSystem(DSMX_11MS_2048);
    } else if (bindMode == Internal_DSMx_22ms ||
               bindMode == External_DSMx_22ms) {
      setSystem(DSMS_22MS_2048);
    }

    // update the internal flag
    if (bindMode == Internal_DSM2_11ms || bindMode == Internal_DSM2_22ms ||
        bindMode == Internal_DSMx_11ms || bindMode == Internal_DSMx_22ms) {
      isInternalFlag = true;
    } else {
      isInternalFlag = false;
    }

    log("-> isInternal:", isInternal() ? "true" : "false");
    logHex("-> system:", system);
  }

  /// @brief Starts bind sequence via power and RX pins.
  void startBinding(unsigned powerPin, unsigned rxPin) {
    // switch off serial interface
    if (serial) {
      log("startBinding");

      pinMode(rxPin, OUTPUT);       // sets the digital pin as output
      digitalWrite(rxPin, LOW);     // make sure that the pin off
      pinMode(powerPin, OUTPUT);    // sets the digital pin as output
      digitalWrite(powerPin, LOW);  // make sure that the pin off
      digitalWrite(rxPin, HIGH);    // set initial state to high
      delay(2000);

      // To put a receiver into bind mode, within 200ms of power application
      // the host device needs to issue a series of falling pulses
      pinMode(powerPin, OUTPUT);     // sets the digital pin as output
      digitalWrite(powerPin, HIGH);  // make sure that the pin off
      delay(50);

      log("-> number of pulses: ", bindMode);

      for (int j = 0; j < bindMode; j++) {
        digitalWrite(rxPin, HIGH);                  // sets the digital pin on
        delayMicroseconds(BINDING_PULSE_DELAY_MS);  // waits
        digitalWrite(rxPin, LOW);                   // sets the digital pin off
        delayMicroseconds(BINDING_PULSE_DELAY_MS);  // waits
      }

      log("-> number of pulses DONE");

      delay(500);
      pinMode(rxPin, INPUT);  // sets the pin as input
    }
  }

  /// @brief Reads and parses one frame from the stream.
  bool getFrame(int transactionTimeMs = DEFAULT_RECEIVING_TIMEOUT) {
    short inByte;
    byte inData[SEND_BUFFER_SIZE];
    bool result = false;
    long available = serial->available();

    // 16-byte data packet every 11ms or 22ms
    if (available >= 16) {
      timeOfLastRead = millis();
      // resynchronize and use last data
      if (!processAllData && available > 16) {
        long diff = available - 16;
        log("skipping number of bytes:", diff);
        // skip unnecessary data
        for (int j = 0; j < diff; j++) serial->read();
      }

      // read the latest data packet
      inByte = serial->readBytes(inData, 16);
      if (inByte != 16) {
        log("We could not read all data");
        result = false;
      } else {
        // check if we processed the data within indicated time period
        result = isConnected(transactionTimeMs);
        if (result) {
          parseFrame(inData);
          // check if the frame is valid
          result = isValidSystem(this->system);
          status = Receiving;

          // log status
          logFrame(available, result);
        } else {
          log("Frame ignored because of timeout");
        }
      }
    }

    return result;
  }

  /// @brief Returns the scaled value of the given channel.
  T getChannelValue(Channel channelId) {
    if (channelId >= Throttle && channelId <= Aux7) {
      return scaler.scale(channelValues[channelId]);
    } else {
      log("Invalid Channel Number:", static_cast<int>(channelId));
      return 0;
    }
  }
  /// @brief Returns the scaled `Throttle` channel value.
  T getThrottle() { return getChannelValue(Throttle); }
  /// @brief Returns the scaled `Aileron` channel value.
  T getAileron() { return getChannelValue(Aileron); }
  /// @brief Returns the scaled `Elevator` channel value.
  T getElevator() { return getChannelValue(Elevator); }
  /// @brief Returns the scaled `Rudder` channel value.
  T getRudder() { return getChannelValue(Rudder); }
  /// @brief Returns the scaled `Gear` channel value.
  T getGear() { return getChannelValue(Gear); }
  /// @brief Returns the scaled `Aux1` channel value.
  T getAux1() { return getChannelValue(Aux1); }
  /// @brief Returns the scaled `Aux2` channel value.
  T getAux2() { return getChannelValue(Aux2); }
  /// @brief Returns the scaled `Aux3` channel value.
  T getAux3() { return getChannelValue(Aux3); }
  /// @brief Returns the scaled `Aux4` channel value.
  T getAux4() { return getChannelValue(Aux4); }
  /// @brief Returns the scaled `Aux5` channel value.
  T getAux5() { return getChannelValue(Aux5); }
  /// @brief Returns the scaled `Aux6` channel value.
  T getAux6() { return getChannelValue(Aux6); }
  /// @brief Returns the scaled `Aux7` channel value.
  T getAux7() { return getChannelValue(Aux7); }

  /// @brief Sets a channel value using the public value type.
  void setChannelValue(Channel channelId, T value) {
    if (channelId >= Throttle && channelId <= Aux7) {
      channelValues[channelId] = scaler.deScale(value);
      if (channelId >= Aux1) {
        isSendAuxData = true;
      }
    } else {
      log("Invalid Channel Number:", static_cast<int>(channelId));
    }
  }
  /// @brief Sets the `Throttle` channel value.
  void setThrottle(T value) { setChannelValue(Throttle, value); }
  /// @brief Sets the `Aileron` channel value.
  void setAileron(T value) { setChannelValue(Aileron, value); }
  /// @brief Sets the `Elevator` channel value.
  void setElevator(T value) { setChannelValue(Elevator, value); }
  /// @brief Sets the `Rudder` channel value.
  void setRudder(T value) { setChannelValue(Rudder, value); }
  /// @brief Sets the `Gear` channel value.
  void setGear(T value) { setChannelValue(Gear, value); }
  /// @brief Sets the `Aux1` channel value.
  void setAux1(T value) { setChannelValue(Aux1, value); }
  /// @brief Sets the `Aux2` channel value.
  void setAux2(T value) { setChannelValue(Aux2, value); }
  /// @brief Sets the `Aux3` channel value.
  void setAux3(T value) { setChannelValue(Aux3, value); }
  /// @brief Sets the `Aux4` channel value.
  void setAux4(T value) { setChannelValue(Aux4, value); }
  /// @brief Sets the `Aux5` channel value.
  void setAux5(T value) { setChannelValue(Aux5, value); }
  /// @brief Sets the `Aux6` channel value.
  void setAux6(T value) { setChannelValue(Aux6, value); }
  /// @brief Sets the `Aux7` channel value.
  void setAux7(T value) { setChannelValue(Aux7, value); }

  /// @brief Sends a binary Spektrum frame.
  void sendData() {
    if (sendCount > 0 && sendCount++ % logMod == 0) {
      log("sendData");
    }
    Data* data = getSendBuffer();
    serial->write((byte*)data, SEND_BUFFER_SIZE);

    // send Aux if necessary
    if (isSendAuxData) {
      Data* aux = getSendBuffer(true);
      serial->write((byte*)aux, SEND_BUFFER_SIZE);
    }
  }
  /// @brief Sends textual data through the stream.
  void sendData(uint8_t* str) {
    if (logMod > 0 && sendCount++ % logMod == 0) {
      log((char*)str);
    }
    serial->print((char*)str);
    serial->flush();
  }

  /// @brief Returns `true` if the link is connected.
  bool isConnected() { return isConnected(TRANSACTION_TIME); }
  /// @brief Returns `true` if connected within the given timeout.
  bool isConnected(long timeoutMs) {
    return (millis() - timeOfLastRead < timeoutMs);
  }

  /// @brief Blocks until at least one byte is available.
  void waitForData() {
    log("waitForData");
    while (!serial->available()) {
      log1(".");
      delay(1000);
    }
  }

  /// @brief Returns the display name of a channel.
  const char* getChannelName(Channel channelId) {
    return ChannelNames[channelId];
  }

  /// @brief Sets output scaling range for channel getters/setters.
  void setChannelValueRange(T min, T max) {
    log("setChannelValueRange");
    rangeMin = min;
    rangeMax = max;
    isChannelRangeConfigured = true;
    scaler.setValues(0, is2048() ? 2048 : 1024, min, max);
    log("setChannelValueRange <-");
  }

  /// @brief Returns direct access to the internal scaler.
  ScalerT* getScaler() { return &this->scaler; }

  /// @brief Sets the protocol system/mode (e.g. 1024 or 2048).
  void setSystem(System system) {
    logHex("setSystem:", system);
    this->system = system;

    if (system == DSM2_22MS_1024) {
      maskCHANID = MASK_1024_CHANID;
      maskVALUE = MASK_1024_SXPOS;
    } else {
      maskCHANID = MASK_2048_CHANID;
      maskVALUE = MASK_2048_SXPOS;
    }

    if (isChannelRangeConfigured) {
      scaler.setValues(0, is2048() ? 2048 : 1024, rangeMin, rangeMax);
    }
  }

  /// @brief Returns the active protocol system.
  System getSystem() { return this->system; }

  /// @brief Validates a system ID against supported values.
  boolean isValidSystem(int system) {
    bool result = false;
    if (isInternal()) {
      if (system == DSM2_22MS_1024 || system == DSM2_11MS_2048 ||
          system == DSMS_22MS_2048 || system == DSMX_11MS_2048) {
        result = true;
      }
      if (!result) {
        log("isValidSystem: ", result ? "true" : "false");
        logHex("system: ", system);
      }
    } else {
      result = true;
    }
    return result;
  }

  /// @brief Returns `true` if current bind mode is internal.
  boolean isInternal() { return this->isInternalFlag; }

  /// @brief Toggles endianness swapping for frame fields.
  void switchEndianness() { this->isSwapBytes = !this->isSwapBytes; }

  /// @brief Returns `true` when using 2048-resolution mode.
  bool is2048() { return this->system == DSM2_22MS_1024 ? false : true; }

  /// @brief Returns the latest fades value from received data.
  uint16_t getFades() { return this->fades; }

  /// @brief Returns current receiver status.
  Status getStatus() { return this->status; }

  /// @brief Enables/disables processing of all buffered bytes.
  void setProcessAllData(bool flag) { processAllData = flag; }

  /// @brief Parses a raw 16-byte frame.
  bool parseFrame(byte* inData) { return parseFrame((Data*)inData); }
  /// @brief Parses a typed frame structure.
  bool parseFrame(Data* inData) {
    Data* data = (Data*)inData;
    // a frame is 16 bytes -> 7 channels + fades
    if (isInternal()) {
      this->fades = data->header.internal.fades;
      System recevedSystem = (System)data->header.internal.system;
      if (!isSystemReported || recevedSystem != getSystem()) {
        logHex("System from the Satellite:", recevedSystem);
        isSystemReported = true;
      }
      if (recevedSystem != getSystem()) {
        if (isValidSystem(recevedSystem))
          setSystem(recevedSystem);
        else
          logHex("Unexpected system", recevedSystem);
      }
    } else {
      this->fades = data->header.fades;
    }

    uint16_t channelShift = is2048() ? 11 : 10;
    for (int i = 0; i < 7; i++) {
      uint16_t inValue = data->values[i];
      swapBytes(&inValue);
      uint16_t channelID = (inValue & maskCHANID) >> channelShift;
      uint16_t channelValue = inValue & maskVALUE;

      if (channelID >= 0 && channelID < MAX_CHANNELS) {
        channelValues[channelID] = channelValue;
      }
    }
    return true;
  }
  /// @brief Builds a frame buffer for sending.
  Data* getSendBuffer(boolean auxData) {
    // Clear only the values array and header
    for (int i = 0; i < 7; ++i) dataPacket.values[i] = 0;
    dataPacket.header.fades = 0;

    uint16_t channelShift = is2048() ? 11 : 10;
    if (auxData) {
      for (int j = 6; j < MAX_CHANNELS && (j - 6) < 7; ++j) {
        uint16_t value = (channelValues[j] & maskVALUE) | (j << channelShift);
        swapBytes(&value);
        dataPacket.values[j - 6] = value;
      }
    } else {
      for (int j = 0; j < 7; ++j) {
        uint16_t value = (channelValues[j] & maskVALUE) | (j << channelShift);
        swapBytes(&value);
        dataPacket.values[j] = value;
      }
    }

    if (isInternal()) {
      dataPacket.header.internal.fades = this->fades;
      dataPacket.header.internal.system = this->system;
    } else {
      dataPacket.header.fades = this->fades;
    }

    return &dataPacket;
  }
  /// @brief Builds the primary frame buffer for sending.
  Data* getSendBuffer() { return getSendBuffer(false); }

  /// @brief Sets the logging output stream.
  void setLog(Stream& logSer) { this->serialLog = &logSer; }
  /// @brief Sets logging interval modulo.
  void setLogMod(long value) { this->logMod = value; }

  /// @brief Returns direct access to raw (unscaled) channel values.
  uint16_t* getChannelValuesRaw() { return channelValues; }

 protected:
  uint16_t channelValues[12];
  Data dataPacket;  //;uint16_t sendValues[7];
  unsigned long timeOfLastRead;
  unsigned long successCount;
  unsigned long failCount;
  unsigned long frameCount;
  unsigned long sendCount;
  uint16_t maskCHANID;
  uint16_t maskVALUE;
  uint16_t fades;
  System system;
  T rangeMin = 0;
  T rangeMax = 100;
  boolean isInternalFlag;
  boolean isSendAuxData;
  boolean isSwapBytes;
  boolean isSystemReported;
  boolean isChannelRangeConfigured;
  boolean processAllData = false;
  Stream* serial;
  Stream* serialLog = NULL;
  ScalerT scaler;
  BindMode bindMode;
  Status status;
  long logMod = 1000;

  /// @brief Logs a message.
  void log(const char* str) {
    if (serialLog == NULL) return;
    serialLog->println(str);
  }
  /// @brief Logs a lightweight heartbeat marker.
  void log1(const char*) {
    if (serialLog == NULL) return;
    serialLog->print(" ");
  }
  /// @brief Logs two strings.
  void log(const char* str, const char* str1) {
    if (serialLog == NULL) return;
    serialLog->print(str);
    serialLog->print(" ");
    serialLog->println(str1);
  }
  /// @brief Logs a string and integer value.
  void log(const char* str, int value) {
    if (serialLog == NULL) return;
    serialLog->print(str);
    serialLog->print(" ");
    serialLog->println(value);
  }
  /// @brief Logs a string and hexadecimal value.
  void logHex(const char* str, int value) {
    if (serialLog == NULL) return;
    serialLog->print(str);
    serialLog->print(" ");
    serialLog->println(value, HEX);
  }

  // private methods
  void logFrame(long available, bool result) {
    if (result) {
      successCount++;
    } else {
      failCount++;
    }
    if (logMod > 0) {
      if (getStatus() == Receiving) {
        if (frameCount % logMod == 0) {
          log("getFrame");
          log("available data:", available);
          log("-> isConnected:", isConnected() ? "true" : "false");
          log("-> isValidSystem:",
              isValidSystem(this->system) ? "true" : "false");
          log("-> frameCount:", frameCount);
          log("-> successCount:", successCount);
          log("-> failCount:", failCount);
        }
      } else {
        if (serialLog != NULL) {
          serialLog->print(available > 0 ? "+" : ".");
        }
      }
    }
    frameCount++;
  }

  void swapBytes(uint16_t* value) {
    if (isSwapBytes) {
      *value = ((*value << 8) & 0xff00) | ((*value >> 8) & 0x00ff);
    }
  }
};

} // namespace spektrum_satellite

#ifdef ARDUINO
using namespace spektrum_satellite;
#endif