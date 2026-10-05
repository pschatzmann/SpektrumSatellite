#pragma once

#include "SpektrumSatellite.h"

namespace spektrum_satellite {

/**
 * @brief CSV serialization helper for `SpektrumSatellite` channel data.
 *
 * This utility converts channel values to a single CSV line and can parse a
 * CSV line back into a `SpektrumSatellite` instance.
 *
 * @tparam T Channel value type used by `SpektrumSatellite` (e.g. `uint16_t`,
 * `float`).
 * @tparam ScalerT Scaler class used by `SpektrumSatellite`.
 */
template <class T, class ScalerT = Scaler<T>>
class SpektrumCSV {
 public:
  /**
   * @brief Construct a CSV serializer/parser.
   *
   * @param delimiter Field separator character used between channels.
   * @param decimals Number of decimals used when formatting values (0-9).
   * @param isTranslated
   *  - `true`: use translated/scaled values via `getChannelValue()` and
   *    `setChannelValue()`.
   *  - `false`: use raw channel values via `getChannelValuesRaw()`.
   */
  SpektrumCSV(char delimiter = ',', int decimals = 2,
              bool isTranslated = true) {
    this->delimiter = delimiter;
    this->isTranslated = isTranslated;
    if (decimals < 0) decimals = 0;
    if (decimals > 9) decimals = 9;
    this->decimals = decimals;
  }

  /**
   * @brief Serialize all channels to CSV.
   *
   * Produces one line with `MAX_CHANNELS` values separated by
   * `delimiter`, terminated by `\n`. The output is truncated if it does not
   * fit into `len` bytes (including the terminating 0).
   *
   * @param satellite Source satellite instance.
   * @param str Output byte buffer receiving the CSV text.
   * @param len Size of `str`.
   */
  void toString(SpektrumSatellite<T, ScalerT>& satellite, uint8_t str[],
                uint16_t len) {
    if (len == 0) return;
    char* out = (char*)str;
    size_t remaining = len;
    out[0] = 0;
    for (int j = 0; j < MAX_CHANNELS; j++) {
      float val = isTranslated ? satellite.getChannelValue((Channel)j)
                               : satellite.getChannelValuesRaw()[j];
      char number[32];
      formatValue(val, number, sizeof(number));
      char separator = j < MAX_CHANNELS - 1 ? delimiter : '\n';
      int n = snprintf(out, remaining, "%s%c", number, separator);
      if (n < 0 || (size_t)n >= remaining) break;  // truncated
      out += n;
      remaining -= n;
    }
  }

  /**
   * @brief Parse CSV channel values and write them into `satellite`.
   *
   * @param str Input CSV line buffer (0 terminated).
   * @param satellite Destination satellite instance.
   * @return `true` if at least one value was parsed, otherwise `false`.
   */
  bool parse(uint8_t* str, SpektrumSatellite<T, ScalerT>& satellite) {
    bool result = false;
    char* start = (char*)str;
    for (int j = 0; j < MAX_CHANNELS; j++) {
      char* end;
      double value = strtod(start, &end);
      if (end == start) break;  // no number found
      result = true;
      if (isTranslated) {
        satellite.setChannelValue((Channel)j, value);
      } else {
        satellite.getChannelValuesRaw()[j] = value;
      }
      if (*end != delimiter) break;  // end of line
      start = end + 1;
    }
    return result;
  }

 private:
  /// Field delimiter used for CSV serialization/parsing.
  char delimiter;
  /// Use scaled values (`true`) or raw values (`false`).
  bool isTranslated;
  /// Number of decimals
  int decimals;

  void formatValue(float value, char* buffer, size_t size) {
#ifdef __AVR__
    // AVR printf does not support %f
    (void)size;
    dtostrf(value, 1, decimals, buffer);
#else
    snprintf(buffer, size, "%.*f", decimals, value);
#endif
  }
};

}  // namespace spektrum_satellite
