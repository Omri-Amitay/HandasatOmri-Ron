#include "Arduino.h"
#include <PID_v1.h>
#include "Motor.h"
#include "Utils.h"
Motor* Motor::instances[2] = {nullptr, nullptr};
Motor::Motor(String motorName, int motorPin, int directionPin, int signalPin, bool reversed)
  : _motorPin(motorPin),
    _directionPin(directionPin),
    _signalPin(signalPin),
    _reversed(reversed),
    _motorName(motorName),
    velocityController(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT){
    
}

void Motor::init() {
  for (int i = 0; i < _totalReadings; i++) { _smoothingArray[i] = 0; }
  pinMode(_motorPin, OUTPUT);
  pinMode(_directionPin, OUTPUT);
  pinMode(_signalPin, INPUT_PULLUP);

  if(instances[0] == nullptr){
    instances[0] = this;
    attachInterrupt(digitalPinToInterrupt(_signalPin), handleInterrupt0, CHANGE);
  }else{
    instances[1] = this;
    attachInterrupt(digitalPinToInterrupt(_signalPin), handleInterrupt1, CHANGE);
  }

  this->setPower(0, 1);
  velocityController.SetOutputLimits(-255, 255);
  velocityController.SetSampleTime(60);  //default is 200ms
  this->velocityController.SetMode(MANUAL);
  // this->velocityController.SetMode(AUTOMATIC);
}

void IRAM_ATTR Motor::handleInterrupt0() {
  if (instances[0] != nullptr) {
    instances[0]->handleSignal();
  }
}

void IRAM_ATTR Motor::handleInterrupt1() {
  if (instances[1] != nullptr) {
    instances[1]->handleSignal();
  }
}

void IRAM_ATTR Motor::handleSignal() {
  if (digitalRead(_signalPin)) {
    // Rising edge
    riseTime = micros();
  } else {
    // Falling edge
    pulseWidth = micros() - riseTime;
    lastPulseTime = micros();
    newPulse = true;
  }
}

double** Motor::getPIDValues() {
  return this->pidValues;
}

float Motor::getFloatMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Motor::setPower(float power, int wantedDirection) {  // true is forward false is backwards
  wantedDirection = wantedDirection <= 0 ? -1 : 1;               //allows -1 to act as negative direction
  

  // direction = _reversed ? direction * -1 : direction; //flips direction
  int ActualDirection = wantedDirection = _reversed ? wantedDirection * -1 : wantedDirection;
  

  this->wantedPower = power * ActualDirection;
  // Serial.print(" P: " + String(this->wantedPower));
  // Serial.println( " D: " + String(ActualDirection));
  ActualDirection = Utils::clamp(wantedDirection, 0, 1);
  power = Utils::inRange(power, 0 , 1000) ? power : power + Ks/2;

  
  analogWrite(_motorPin, Utils::clamp(abs(power), 0, 255));
  digitalWrite(_directionPin, ActualDirection);
}

long Motor::getRawSignal() {

  // unsigned long highPulse = pulseIn(_signalPin, HIGH, _slowestPulse);

  // if (highPulse < _fastestPulse) {
  //   highPulse = _slowestPulse;
  // }
  long pulse = -1;
  unsigned long now = micros();
  unsigned lastSignalTime;
  noInterrupts();
  if(this->newPulse){
    this->newPulse = false;
    pulse = pulseWidth;
  }
  lastSignalTime = this->lastPulseTime;
  interrupts();
  if(now - lastSignalTime > _slowestPulse){
    return _slowestPulse;
  }
  
  return pulse;
}

unsigned long Motor::getFilteredSignal() {

  

  unsigned long highPulse = Motor::getRawSignal();

  if(highPulse != -1){
    _total -= _smoothingArray[readIndex];
    _smoothingArray[readIndex] = highPulse;

    /*Serial.print(" AddingValue: " + String(smoothingArray[readIndex]));*/

    _total += _smoothingArray[readIndex];
    

    /* Serial.print("Average Sample: " + String(average) + " Current Index: " + String(readIndex) + " Total: " + String(total) + " ");
    Serial.print(" [");
    for (int i = 0; i < totalReadings; i++) { Serial.print(String(smoothingArray[i]) + ", "); }
    Serial.print("]");
    */
    readIndex = (readIndex + 1) % _totalReadings;

    if (readIndex == 9) {
      _valueSmoothed = true;
    }

    
  }
  float average = _total / _totalReadings;
  return average;
}

float Motor::getCalculatedRPM() {
  float filteredSignal = Motor::getFilteredSignal();
  // Serial.print(" Sig:");
  // Serial.print(filteredSignal);

  return filteredSignal == this->_slowestPulse ? 0 : 26883/filteredSignal * this->currentDirection;
}

float Motor::calculateOutput() {


  double ff = Ks * Utils::sign(Setpoint) + Kv * Setpoint;
  this->velocityController.Compute();
  double pidOutput = this->Output;
  // Serial.print(" ff: ");
  // Serial.print(ff);
  // Serial.print(" pid: ");
  // Serial.print(pidOutput);
  double totalPower = this->Setpoint == 0 ? 0 : Utils::clamp(ff + pidOutput, -255, 255);
  return totalPower;
}

void Motor::setVelControl(bool enabled){
  if(!enabled){
    this->velocityController.SetMode(MANUAL);
  }
  this->velocityControlled = enabled;
}

void Motor::setVelocity(float vel) {
  this->Setpoint = vel;
}

void Motor::setTunableNumber(double val, double val2) {
  Kd = val;
  // Kd = val2;
  this->velocityController.SetTunings(Kp, Ki, Kd);
}
void Motor::updatePID(double* newValues[6]) {
  for (int i = 0; i < 6; i++) {
    *(this->pidValues[i]) = *(newValues[i]);
  }
  this->velocityController.SetTunings(Kp, Ki, Kd);

  Serial.print(_motorName);
  Serial.print(" Kp: ");
  Serial.print(Kp);
  Serial.print(" Ki: ");
  Serial.print(Ki);
  Serial.print(" Kd: ");
  Serial.print(Kd);
}


/*double timeSinceChange = millis();
double lastRpmValue = rpmOld
changedvalue = false
decreased

if sign lastrpm != power
changedValue = true

if ChangedValue
  if !decreased
    decreased = currentRpm < lastRpm
  else
    changedValue = currentRPM > lastRPM

motorDirection = if changedValue then sign power else motorDirection
*/

void Motor::update() {
  this->Input = this->getCalculatedRPM();
  
  float power = this->calculateOutput();
  

  if(Utils::sign(Input) != Utils::sign(power)){
    changedSign = true;
  }

  if(changedSign){
    if(!decreased){
      decreased = this->Input < this->lastRPM;
    }else{
      changedSign = this->Input > this->lastRPM;
    }
    if(this->lastSwitchTime + switchTimeMax < millis()){
      changedSign = false;
      decreased = false;
    }
  }

  if(!changedSign){
    this->currentDirection = Utils::sign(this->wantedPower);
    changedSign = false;
    decreased = false;
  }


  // Serial.print(" CG: " + String(changedSign) + " D: " + String(decreased) + " LRPM: " + String(this->lastRPM) + " IN: " + String(this->Input) + " WP: " + String(this->wantedPower) + " CD: " + String(this->currentDirection));


  // this->setPower(abs(power), Utils::sign(this->currentDirection));
  this->lastRPM = this->Input;
  

}

void Motor::print(){
  Serial.print(_motorName + " R: " + String(Input));
  Serial.print( " P: " + String(this->wantedPower));
}
