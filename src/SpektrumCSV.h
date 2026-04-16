#pragma once

#include "SpektrumSatellite.h"

/**
 * @brief CSV serialization helper for `SpektrumSatellite` channel data.
 *
 * This utility converts channel values to a single CSV line and can parse a
 * CSV line back into a `SpektrumSatellite` instance.
 *
 * @tparam T Channel value type used by `SpektrumSatellite` (e.g. `uint16_t`,
 * `float`).
 */
template <class T>
class SpektrumCSV {
 public:
  /**
   * @brief Construct a CSV serializer/parser.
   *
   * @param delimiter Field separator character used between channels.
   * @param decimals Number of decimals used when formatting values.
   * @param isTranslated
   *  - `true`: use translated/scaled values via `getChannelValue()` and
   *    `setChannelValue()`.
   *  - `false`: use raw channel values via `getChannelValuesRaw()`.
   */
  SpektrumCSV(char delimiter = ',', int decimals = 2,
              bool isTranslated = true) {
    this->delimiter = delimiter;
    this->isTranslated = isTranslated;
    // determine format string e.g. "%.2f"
    strcpy(format, "%.");
    char decimalsTxt[10];
    itoa(decimals, decimalsTxt, 10);
    strcat(format, decimalsTxt);
    strcat(format, "f");
  }

  /**
   * @brief Serialize all channels to CSV.
   *
   * Produces one line with `MAX_CHANNELS` values separated by
   * `delimiter`, terminated by `\n`.
   *
   * @param satellite Source satellite instance.
   * @param dataSting Output byte buffer receiving the CSV text.
   * @param maxLen Maximum available size of `dataSting`.
   */
  void toString(SpektrumSatellite<T>& satellite, uint8_t str[], uint16_t len) {
    uint8_t* start = str;
    for (int j = 0; j < MAX_CHANNELS; j++) {
      float val = isTranslated ? satellite.getChannelValue((Channel)j)
                               : satellite.getChannelValuesRaw()[(Channel)j];
      int len = sprintf((char*)start, format, val);
      start += len;
      if (j < MAX_CHANNELS - 1) {
        *start = delimiter;
        start++;
      }
    }
    sprintf((char*)start, "\n");
  }

  /**
   * @brief Parse CSV channel values and write them into `satellite`.
   *
   * @param str Input CSV line buffer.
   * @param satellite Destination satellite instance.
   * @return `true` if at least one value was parsed, otherwise `false`.
   */
  bool parse(uint8_t* str, SpektrumSatellite<T>& satellite) {
    bool result = false;
    char* start = (char*)str;
    for (int j = 0; j < MAX_CHANNELS; j++) {
      Channel ch = (Channel)j;
      char* end = findEnd(start);
      if (end == NULL) {
        break;
      }
      result = true;
      double value = strtod(start, &end);
      if (isTranslated) {
        satellite.setChannelValue(ch, value);
      } else {
        satellite.getChannelValuesRaw()[j] = value;
      }
      start = end + 1;
    }
    return result;
  }

  /**
   * @brief Set an optional value scaling factor.
   *
   * @note This function is declared but not implemented in this header.
   */
  void setFactor(double factor);

 private:
  /// Field delimiter used for CSV serialization/parsing.
  char delimiter;
  /// Use scaled values (`true`) or raw values (`false`).
  bool isTranslated;
  /// `sprintf` format string, e.g. `"%.2f"`.
  char format[15];

  /**
   * @brief Find end-of-field marker in a CSV line.
   * @param start Pointer to current field start.
   * @return Pointer to delimiter/newline/end marker or `NULL` if not found.
   */
  char* findEnd(char* start) {
    char* end = strchr(start, delimiter);
    if (end == NULL) {
      end = strchr(start, '\n');
    }
    return end;
  }
};
