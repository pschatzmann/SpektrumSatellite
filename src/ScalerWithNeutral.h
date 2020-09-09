/**
 * Sometimes the neutral input position is not exactly the the middle of min and max. We allow the definition
 * of a non central netral position:
 * 
 * e.g. a Joystick sends values between 0 and 100 - but if the joystick is not touched we receive 55 
 * 
 */

#ifndef NEUTRALSCALER_H_
#define NEUTRALSCALER_H_

#include "SpektrumSatellite.h"

// forward declaration of SpektrumSatellite
template <class T> class SpektrumSatellite; 

template <class T> class ScalerWithNeutral {
  public:
    Scaler();
    void setValues(T inMin, T inMax,T inNeutral, T outMin,T outMax);

    T scale(uint16_t value);
    uint16_t deScale(T value);
    T neutral;


  private:
    Scaler lowScaler;
    Scaler highScaler;
    T neutral;
    T neutralTo;
};

template <class T>
Scaler<T>::Scaler(){
}

template <class T>
void Scaler<T>::setValues(T fromMin, T fromMax,T fromNeutral, T toMin, T toMax) {
  this->neutral = neutral;
  this->neutralTo = toMax-toMin) /2.0;
  lowScaler.setValues(fromMin, fromNeutral, toMin, neutralTo )
  highScaler.setValues(fromNeutral, fromMax, neutralTo, toMax )
}


template <class T>
T Scaler<T>::scale(uint16_t value) {
  return (value<=neutral) ? lowScaler.scale(value) : highScaler.scale(value);
}


template <class T>
uint16_t Scaler<T>::deScale(T value) {
  return (value<=neutralTo) ? lowScaler.deScale(value) : highScaler.deScale(value);
}

#endif /* NEUTRALSCALER_H_ */
