#include "Arduino.h"
#include "Motor.h"

Motor::Motor(int motorPin, int directionPin, int signalPin, bool reversed){
  _motorPin = motorPin;
  _directionPin = directionPin;
  _signalPin = signalPin;
  _reversed = reversed;
}
