
#ifndef Gyro_h
#define Gyro_h

#include <Arduino.h>

#include <MPU6050_light.h>
#include <Wire.h>

class Gyro{
  public:
    Gyro();
    void init();
    float getPitch();
    float getYaw();
    void update();

    void CalibrateGyro();
  private:
   
    float ALPHA = 0.75;
    float angleY = 0;
    float gyroBiasY = 0;
    unsigned long lastMicros;
};

#endif
