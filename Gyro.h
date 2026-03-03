
#ifndef Gyro_h
#define Gyro_h

#include <Arduino.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

class Gyro{
  public:
    Gyro();

    float getY();
    void update();
  private:
   
    float ALPHA = 0.75;
    float angleY = 0;
    float gyroBiasY = 0;
    unsigned long lastMicros;
};

#endif
