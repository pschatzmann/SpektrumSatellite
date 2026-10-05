#pragma once

#include "Scaler.h"

namespace spektrum_satellite {

/**
 * @brief Sometimes the neutral input position is not exactly the the middle of
 * min and max. We allow the definition of a non central neutral position:
 *
 * e.g. a Joystick sends values between 0 and 100 - but if the joystick is not
 * touched we receive 55
 *
 * Can be used as ScalerT of SpektrumSatellite: call setNeutral() via
 * getScaler() to define the neutral input position.
 * @author Phil Schatzmann
 */
template <class T>
class ScalerWithNeutral {
 public:
  ScalerWithNeutral() = default;

  /// Defines the ranges: the neutral input position is the middle of the input
  /// range unless it was defined with setNeutral()
  void setValues(T fromMin, T fromMax, T toMin, T toMax) {
    T fromNeutral = hasNeutral ? neutral : fromMin + (fromMax - fromMin) / 2;
    setValues(fromMin, fromMax, fromNeutral, toMin, toMax);
  }

  /// Defines the ranges with an explicit neutral input position
  void setValues(T fromMin, T fromMax, T fromNeutral, T toMin, T toMax) {
    this->fromMin = fromMin;
    this->fromMax = fromMax;
    this->toMin = toMin;
    this->toMax = toMax;
    this->neutral = fromNeutral;
    this->hasNeutral = true;
    this->isConfigured = true;
    this->neutralTo = toMin + (toMax - toMin) / 2;
    lowScaler.setValues(fromMin, fromNeutral, toMin, neutralTo);
    highScaler.setValues(fromNeutral, fromMax, neutralTo, toMax);
  }

  /// Defines the neutral input position
  void setNeutral(T fromNeutral) {
    neutral = fromNeutral;
    hasNeutral = true;
    if (isConfigured) {
      setValues(fromMin, fromMax, fromNeutral, toMin, toMax);
    }
  }

  T scale(float value) {
    return (value <= neutral) ? lowScaler.scale(value)
                              : highScaler.scale(value);
  }

  float deScale(T value) {
    return (value <= neutralTo) ? lowScaler.deScale(value)
                                : highScaler.deScale(value);
  }

 private:
  Scaler<T> lowScaler;
  Scaler<T> highScaler;
  T fromMin = 0, fromMax = 0, toMin = 0, toMax = 0;
  T neutral = 0;
  T neutralTo = 0;
  bool hasNeutral = false;
  bool isConfigured = false;
};

}  // namespace spektrum_satellite
