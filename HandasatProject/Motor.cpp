#include "Arduino.h"
#include <PID_v1.h>
#include "Motor.h"
#include "Utils.h"

Motor::Motor(String motorName, int motorPin, int directionPin, int signalPin, bool reversed) 
  :_motorPin(motorPin),
  _directionPin(directionPin),
  _signalPin(signalPin),
  _reversed(reversed),
  _motorName(motorName), 
  velocityController(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT)
{}

void Motor::init() {
  for (int i = 0; i < _totalReadings; i++) { _smoothingArray[i] = 0; }
  pinMode(_motorPin, OUTPUT);
  pinMode(_directionPin, OUTPUT);
  pinMode(_signalPin, INPUT_PULLUP);
  this->setPower(0, 1);
  velocityController.SetOutputLimits(-255,255);
  velocityController.SetSampleTime(60); //default is 200ms
  this->velocityController.SetMode(MANUAL);
  this->velocityController.SetMode(AUTOMATIC);
  
}

float Motor::getFloatMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Motor::setPower(float power, int direction) { // true is forward false is backwards
  direction = direction < 0 ? -1 : 1; //allows -1 to act as negative direction
  this->currentDirection = direction;

  // direction = _reversed ? direction * -1 : direction; //flips direction
  direction = _reversed ? direction * -1 : direction;
  direction = Utils::clamp(direction, 0, 1);
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
  float filteredSignal = Motor::getFilteredSignal();
  return filteredSignal == this->_slowestPulse ? 0 : 26883 * pow(filteredSignal, -1) * this->currentDirection;
}

float Motor::calculateOutput(){


  double ff = Ks * Utils::sign(Setpoint) + Kv * Setpoint; 
  this->velocityController.Compute();
  double pidOutput = this->Output;
  // Serial.print(" ff: ");
  // Serial.print(ff);
  // Serial.print(" pid: ");
  // Serial.print(pidOutput);
  double totalPower = this->Setpoint==0 ? 0 : Utils::clamp(ff + pidOutput, -255, 255);
  return totalPower;
}

void Motor::setVelocity(float vel){

  this->Setpoint = vel;
}

void Motor::setTunableNumber(double val, double val2){
  Kd = val;
  // Kd = val2;
  this->velocityController.SetTunings(Kp,Ki,Kd);

}
void Motor::update() {
  this->Input = this->getCalculatedRPM();
  float power = this->calculateOutput();
  this->setPower(abs(power), Utils::sign(power));

  // float rpm = this->getCalculatedRPM();  //this causes the signal list to keep updating resulting in new RPM values.
   Serial.print(_motorName + "Setpoint: ");
  Serial.print(this->Setpoint);
  // Serial.print(", Power: ");
  // Serial.print(",");
  // Serial.print(power);
  Serial.print(", ActualVel: ");
  Serial.print(",");
  Serial.print(this->Input);
  /*Serial.print(", Signal: ");
  Serial.print(",");
  // Serial.print(this->Kp);
  // Serial.print(",");
  // Serial.print(this->Kd);


  // Serial.print(_motorName + "RPM: ");
  // Serial.print(rpm); */
}
