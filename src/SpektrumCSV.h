/**
 *  Serialization of the SpektrumSatellite data to and from a CSV line
 */

#ifndef SPEKTRUMCSV_H_
#define SPEKTRUMCSV_H_

#include "SpektrumSatellite.h"

// forward declaration of SpektrumSatellite
template <class T> class SpektrumSatellite; 

template <class T> 
class SpektrumCSV {
    public:
        SpektrumCSV(char delimiter=',', bool isTranslated=true);
        void toString(SpektrumSatellite<T> &satellite, uint8_t dataSting[], uint16_t maxLen);
        bool parse(uint8_t* str, SpektrumSatellite<T> &satellite);
        void setFactor(double factor);
    private:
      char delimiter;
      bool isTranslated;
      char* findEnd(char* start);
};

template <class T>
SpektrumCSV<T>::SpektrumCSV(char delimiter, bool isTranslated){
    this->delimiter = delimiter;
    this->isTranslated = isTranslated;
}


/**
 * Convert to tab seperated values
 */
template <class T>
void SpektrumCSV<T>::toString(SpektrumSatellite<T> &satellite, uint8_t str[], uint16_t len) {
    uint8_t* start = str;
    for (int j=0; j < MAX_CHANNELS; j++){
        float val = isTranslated ?  satellite.getChannelValue((Channel)j): satellite.getChannelValuesRaw()[(Channel)j];
        int len = sprintf((char*)start, "%.2f", val);
        start+=len;
        if (j<MAX_CHANNELS-1){
            *start = delimiter;
            start++;
        }
    }
    sprintf((char*)start,"\n");
}

/**
 * Parse tab seperated values
 */
template <class T>
bool SpektrumCSV<T>::parse(uint8_t* str, SpektrumSatellite<T> &satellite){
    bool result = false;
    char* start = (char*)str;
    for (int j=0; j< MAX_CHANNELS; j++){
        Channel ch = (Channel) j;
        char* end = findEnd(start);
        if (end==NULL){
            break;
        }
        result = true;
        double value = strtod(start, &end);
        if (isTranslated){
            satellite.setChannelValue(ch, value);
        } else {
            satellite.getChannelValuesRaw()[j] = value;
        }
        start = end+1;
    }
    return result;
}

template <class T>
char* SpektrumCSV<T>::findEnd(char* startU) {
    char* start = (char*) startU;
    char* end = strchr (start, delimiter );
    if (end==NULL){
        end = strchr (start, '\n' );
    } else if (end==NULL){
        end = strchr (start, NULL );
    }
    return end;
}



#endif /* SERIALIZATION_H_ */
