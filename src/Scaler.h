#pragma once

#include <type_traits>

#include "SpektrumSatellite.h"

/**
 * @class Scaler
 * @brief Scale from and to defined range. 
 * 
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
      value = map((T)value, inMin, inMax, outMin, outMax);
    }
    return value;
  }

  float deScale(T value) {
    if (this->active) {
      value = map(value, outMin, outMax, inMin, inMax);
    }
    return value;
  }

 private:
  bool active = false;
  T inMin, inMax, outMin, outMax;

  template <typename U = T>
  typename std::enable_if<std::is_floating_point<U>::value, U>::type finalize(
      U value) {
    return round(value);
  }

  template <typename U = T>
  typename std::enable_if<!std::is_floating_point<U>::value, U>::type finalize(
      U value) {
    return value;
  }

  T map(T value, T fromMin, T fromMax, T toMin, T toMax) {
    if (fromMax == fromMin) return toMin;
    T r =
        ((value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin);
    return finalize(r);
  }
};
