#ifndef Motor_h
#define Motor_h

#include "Arduino.h"
#include <PID_v1.h>
class Motor
{
public:
  Motor(String motorName, int motorPin, int directionPin, int signalPin, bool reversed);
  void init();
  void setMotorCalibrationMode(bool isEnabled);

  void setPower(float power, int direction);

  long getRawSignal();
  unsigned long getFilteredSignal();
  float getCalculatedRPM();
  void setVelocity(float vel);
  void setTunableNumber(double val, double val2);
  double **getPIDValues();
  void setVelControl(bool enabled);
  void updatePID(double *newValues[6]);
  void update();
  void print();

private:
  bool checkDirection(float power);
  float getFloatMap(float x, float in_min, float in_max, float out_min, float out_max);
  float calculateOutput();

  static Motor *instances[2];

  static void IRAM_ATTR handleInterrupt0();
  static void IRAM_ATTR handleInterrupt1();

  void IRAM_ATTR handleSignal();

  String _motorName;
  int _motorPin;
  int _directionPin;
  int _signalPin;
  bool _reversed;

  bool velocityControlled = false;

  int currentDirection = 0;

  unsigned long lastSwitchTime = 0;
  long switchTimeMax = 628;
  long lastRPM = 0;

  volatile unsigned long riseTime = 0;
  volatile unsigned long pulseWidth = 0;
  volatile bool newPulse = false;
  volatile unsigned long lastPulseTime = 0;

  bool _motorCalibrationState;
  int readIndex = 0;
  long _slowestPulse = 27000;
  long _fastestPulse = 420;
  unsigned long _total = 0;
  bool _valueSmoothed = false;

  float motorPower = 0;

  double Setpoint, Input, Output;
  double Kp = 50, Ki = 6, Kd = 0;
  double Kv = 5.8, Ks = 3.9;
  PID velocityController;
  double *pidValues[6] = {&Kp, &Ki, &Kd, &Ks, &Kv, &Setpoint};

  const int _totalReadings = 10;
  long _smoothingArray[10];
};

#endif