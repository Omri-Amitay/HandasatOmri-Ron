#ifndef Motor_h
#define Motor_h

#include "Arduino.h"

class Motor{
  public:

    Motor(int motorPin, int directionPin, int signalPin, bool reversed);
    void begin();
    void setMotorCalibrationMode(bool isEnabled);

    void setPower(int power, int direction);

    unsigned long getRawSignal();
    unsigned long getFilteredSignal();
    float getCalculatedRPM();

  private:

    int _motorPin;
    int _directionPin;
    int _signalPin;
    bool _reversed;

    bool _motorCalibrationState;

    const int _totalReadings = 10;
    long _smoothingArray[10];

};


#endif