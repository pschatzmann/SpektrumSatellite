#pragma once

namespace spektrum_satellite {

/**
 * @class Scaler
 * @brief Scale from and to defined range.
 *
 * The Scaler<T> class provides functionality to scale values from one range to
 * another, and to reverse the scaling (de-scale). It is designed to be used
 * with numeric types (such as int, float, or double) and is useful for mapping
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

  /// Scales from the input range to the output range
  T scale(float value) {
    if (this->active) {
      value = map(value, inMin, inMax, outMin, outMax);
    }
    return finalize(value);
  }

  /// Scales from the output range back to the input range (not rounded)
  float deScale(T value) {
    if (this->active) {
      return map(value, outMin, outMax, inMin, inMax);
    }
    return value;
  }

 private:
  bool active = false;
  T inMin, inMax, outMin, outMax;

  /// Rounds the result for integral types, keeps the fraction otherwise
  static T finalize(float value) {
    bool isIntegral = (T)0.5f == (T)0;
    if (isIntegral) {
      value = value < 0 ? value - 0.5f : value + 0.5f;
    }
    return (T)value;
  }

  /// Calculation is done in float to avoid integer overflow and truncation
  static float map(float value, float fromMin, float fromMax, float toMin,
                   float toMax) {
    if (fromMax == fromMin) return toMin;
    return (value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin;
  }
};

}  // namespace spektrum_satellite
