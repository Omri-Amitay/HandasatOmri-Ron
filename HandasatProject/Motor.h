#ifndef Motor_h
#define Motor_h

#include "Arduino.h"

class Motor{
  public:

    Motor(int motorPin, int directionPin, int signalPin, bool reversed);
    void init();
    void setMotorCalibrationMode(bool isEnabled);

    void setPower(float power, int direction);

    unsigned long getRawSignal();
    unsigned long getFilteredSignal();
    float getCalculatedRPM();
    void update();

  private:
    unsigned getFloatMap(float x, float in_min, float in_max, float out_min, float out_max);
    int _motorPin;
    int _directionPin;
    int _signalPin;
    bool _reversed;

    bool _motorCalibrationState;
    int readIndex = 0;
    long _slowestPulse = 18000;
    long _fastestPulse = 420;
    unsigned long _total = 0;
    bool _valueSmoothed = false;

    const int _totalReadings = 10;
    long _smoothingArray[10];

};


#endif