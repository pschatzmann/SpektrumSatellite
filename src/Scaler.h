#pragma once

#include "SpektrumSatellite.h"

/**
 * The Scaler<T> class provides functionality to scale values from one range to
 * another, and to reverse the scaling (de-scale). It is designed to be used
 * with numeric types (such as int, float, or float) and is useful for mapping
 * input values (e.g., sensor readings) to output ranges (e.g., actuator
 * commands).
 * @author Phil Schatzmann
 */
template <class T>
class Scaler {
 public:
  Scaler() = default;

  void setValues(T fromMin, T fromMax, T toMin, T toMax) {
    this->inMin = fromMin;
    this->inMax = fromMax;
    this->outMin = toMin;
    this->outMax = toMax;
    this->active = true;
  }

  T getInMax() { return this->inMax; }

  T getOutMax() { return this->outMax; }

  void setActive(bool active) { this->active = active; }

  bool isActive() { return this->active; }

  T scale(float value) {
    if (this->active) {
      // prevent NPE
      if (outMax == outMin) return 0;
      // (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
      float r =
          (((float)value - (float)inMin) * ((float)outMax - (float)outMin) /
               ((float)inMax - (float)inMin) +
           (float)outMin);
      value = round(r);
    }
    return value;
  }

  float deScale(T value) {
    if (this->active) {
      // prevent NPE
      if (outMax == outMin) return 0;
      float result =
          (((float)value - (float)outMin) * ((float)inMax - (float)inMin) /
               ((float)outMax - (float)outMin) +
           (float)inMin);
      value = round(result);
    }
    return value;
  }

 private:
  bool active = false;
  T inMin, inMax, outMin, outMax;
};

