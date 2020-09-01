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
        SpektrumCSV();
        SpektrumCSV(char delimiter);
        void toString(SpektrumSatellite<T> &satellite, uint8_t* dataSting, unsigned int maxLen);
        void parse(uint8_t* str, SpektrumSatellite<T> &satellite);
        void setFactor(double factor);
    private:
      char delimiter;
      char* findEnd(char* start);
};

template <class T>
SpektrumCSV<T>::SpektrumCSV(){
    this->delimiter = ',';
}

template <class T>
SpektrumCSV<T>::SpektrumCSV(char delimiter){
    this->delimiter = delimiter;
}


/**
 * Convert to tab seperated values
 */
template <class T>
void SpektrumCSV<T>::toString(SpektrumSatellite<T> &satellite, uint8_t* str, unsigned int len) {
    uint8_t* start = str;
    for (int j=0; j < MAX_CHANNELS; j++){
        float val = satellite.getChannelValue((Channel)j);
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
void SpektrumCSV<T>::parse(uint8_t* str, SpektrumSatellite<T> &satellite){
    char* start = (char*)str;
    for (int j=0; j< MAX_CHANNELS; j++){
        char* end = findEnd(start);
        if (end==NULL){
            break;
        }
        double value = strtod(start, &end);
        satellite.setChannelValue((Channel)j, value);
        start = end+1;
    }
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
