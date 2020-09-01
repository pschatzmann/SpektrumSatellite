/**
 * Sacle values to indicated range (if necessary)
 */

#ifndef SCALER_H_
#define SCALER_H_

#include "SpektrumSatellite.h"

// forward declaration of SpektrumSatellite
template <class T> class SpektrumSatellite; 

template <class T> class Scaler {
  public:
    Scaler();
    void setValues(T inMin, T inMax,T outMin,T outMax);
    T getInMax();
    T getOutMax();
    void setActive(bool active);
    bool isActive();

    T scale(uint16_t value);
    uint16_t deScale(T value);


  private:
    bool active;
    bool isRounding;
    T inMin,inMax,outMin,outMax;

};

template <class T>
Scaler<T>::Scaler(){
  active = false;
  T i1 = 1;
  T i2 = 3;
  T result = (i1 / i2) * i2;
  isRounding =  (1.0 - result > 0.1);
}

template <class T>
void Scaler<T>::setValues(T fromMin, T fromMax, T toMin, T toMax) {
   this->inMin = fromMin;
   this->inMax = fromMax;
   this->outMin = toMin;
   this->outMax = toMax;
   this->active = true;
}


template <class T>
T Scaler<T>::getInMax() {
  return this->inMax;
}

template <class T>
T Scaler<T>::getOutMax(){
  return this->outMax;
}

template <class T>
void Scaler<T>::setActive(bool active) {
  this->active = active;
}

template <class T>
bool Scaler<T>::isActive(){
  return this->active;
}


template <class T>
T Scaler<T>::scale(uint16_t value) {
  if (this->active) {
      // (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
      double r = (((double)value - (double)inMin) * ((double)outMax - (double)outMin) / ((double)inMax - (double)inMin) + (double)outMin);
      value = isRounding ? round(r) : r;
  }
  return value;
}


template <class T>
uint16_t Scaler<T>::deScale(T value) {
  if (this->active) {
      double r = (((double)value - (double)outMin) * ((double)inMax - (double)inMin) / ((double)outMax - (double)outMin) + (double)inMin);
      value = isRounding ? round(r) : r;
  }
  return value;
}

#endif /* SCALER_H_ */
