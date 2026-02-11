#pragma once

#include "SpektrumSatellite.h"

/**
 * Sometimes the neutral input position is not exactly the the middle of min and
 * max. We allow the definition of a non central netral position:
 *
 * e.g. a Joystick sends values between 0 and 100 - but if the joystick is not
 * touched we receive 55
 * @author Phil Schatzmann
 */
template <class T>
class ScalerWithNeutral {
 public:
  ScalerWithNeutral() = default;

  void Scaler<T>::setValues(T fromMin, T fromMax, T fromNeutral, T toMin,
                            T toMax) {
    this->neutral = fromNeutral;
    this->neutralTo = (toMax - toMin) / 2.0;
    lowScaler.setValues(fromMin, fromNeutral, toMin, neutralTo);
    highScaler.setValues(fromNeutral, fromMax, neutralTo, toMax);
  }

  T Scaler<T>::scale(uint16_t value) {
    return (value <= neutral) ? lowScaler.scale(value)
                              : highScaler.scale(value);
  }

  uint16_t Scaler<T>::deScale(T value) {
    return (value <= neutralTo) ? lowScaler.deScale(value)
                                : highScaler.deScale(value);
  }

 private:
  Scaler lowScaler;
  Scaler highScaler;
  T neutral;
  T neutralTo;
};
