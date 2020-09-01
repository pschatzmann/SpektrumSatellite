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
    T inMin,inMax,outMin,outMax;
};

template <class T>
Scaler<T>::Scaler(){
  active = false;
}

template <class T>
void Scaler<T>::setValues(T fromMin, T fromMax, T toMin, T toMax) {
   this->inMin = fromMin;
   this->inMax = fromMax;
   this->outMin = toMin;
   this->outMax = toMax;
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
    if (this->inMin!=this->outMin || this->inMax != this->outMax){
      //return (value - this->inMin) / (this->inMax - this->inMin) *  (this->outMax - this->outMin) + this->outMin;
      return map(value, this->inMin, this->inMax, this->outMin, this->outMax);
    } 
  }
  return value;
}


template <class T>
uint16_t Scaler<T>::deScale(T value) {
  if (this->active) {
    if (this->inMin!=this->outMin || this->inMax != this->outMax){
      //return (value - this->inMin) / (this->inMax - this->inMin) *  (this->outMax - this->outMin) + this->outMin;
      return map(value, this->outMin, this->outMax, this->inMin, this->inMax);
    } 
  }
  return value;
}


#endif /* SCALER_H_ */
