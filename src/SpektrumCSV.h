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
        void toString(SpektrumSatellite<T> &data, uint8_t* str, unsigned int len);
        void parse(uint8_t* str, SpektrumSatellite<T> *satellite);
        void setFactor(double factor);
    private:
      char delimiter;
      double factor = 1.0;
      uint8_t* findEnd(uint8_t* start);
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
        int len = sprintf((char*)start, "%f",val);
        start+=len;
    }
    sprintf((char*)start,"\n");
}

/**
 * Parse tab seperated values
 */
template <class T>
void SpektrumCSV<T>::parse(uint8_t* str, SpektrumSatellite<T> *satellite){
    uint8_t* start = str;
    for (int j=0; j< MAX_CHANNELS; j++){
        uint8_t* end = findEnd(start);
        if (end==NULL){
            break;
        }
        double value = strtod((char*)start, &end);
        satellite.setChannelValue((Channel)j, value);
        start = end+1;
    }
}

template <class T>
uint8_t* SpektrumCSV<T>::findEnd(uint8_t* start) {
    char* end = strchr ((char*) start, '\t' );
    if (end==NULL){
        char* end = strchr ((char*) start, '\n' );
    } else if (end==NULL){
        char* end = strchr ((char*) start, NULL );
    }
    return end;
}



#endif /* SERIALIZATION_H_ */
