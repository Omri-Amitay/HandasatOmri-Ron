#include "Arduino.h"
#include "Motor.h"

Motor::Motor(int motorPin, int directionPin, int signalPin, bool reversed) {
  _motorPin = motorPin;
  _directionPin = directionPin;
  _signalPin = signalPin;
  _reversed = reversed;
}

void Motor::init() {
  for (int i = 0; i < _totalReadings; i++) { _smoothingArray[i] = 0; }
  pinMode(_motorPin, OUTPUT);
  pinMode(_directionPin, OUTPUT);
  pinMode(_signalPin, INPUT_PULLUP);
}

unsigned Motor::getFloatMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Motor::setPower(float power, int direction) {
  direction
  analogWrite(_motorPin, power);
  digitalWrite(_directionPin, direction);
}

unsigned long Motor::getRawSignal() {
  unsigned long highPulse = pulseIn(_signalPin, HIGH, _slowestPulse);

  if (highPulse < _fastestPulse) {
    highPulse = _slowestPulse;
  }
  return highPulse;
}

unsigned long Motor::getFilteredSignal() {

  _total -= _smoothingArray[readIndex];

  unsigned long highPulse = Motor::getRawSignal();

  _smoothingArray[readIndex] = highPulse;

  /*Serial.print(" AddingValue: " + String(smoothingArray[readIndex]));*/

  _total += _smoothingArray[readIndex];
  float average = _total / _totalReadings;

  /* Serial.print("Average Sample: " + String(average) + " Current Index: " + String(readIndex) + " Total: " + String(total) + " ");
  Serial.print(" [");
  for (int i = 0; i < totalReadings; i++) { Serial.print(String(smoothingArray[i]) + ", "); }
  Serial.print("]");
  */
  readIndex = (readIndex + 1) % _totalReadings;

  if (readIndex == 9) {
    _valueSmoothed = true;
  }

  return average;
}

float Motor::getCalculatedRPM() {
  return 26883 * pow(Motor::getFilteredSignal(), -1);
}

void Motor::update() {
  Motor::getFilteredSignal();  //this causes the signal list to keep updating resulting in new RPM values.
}
