
#include "Gyro.h"
#include "Arduino.h"

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

Gyro::Gyro(){
  if (!mpu.begin()) while (1);

mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // --- CALIBRATION STEP ---
  // Hold the robot perfectly still and upright!
  Serial.println("Calibrating... Keep upright.");
  float sumGyroY = 0;
  for (int i = 0; i < 200; i++) {
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);
    sumGyroY += g.gyro.y;
    delay(5);
  }
  gyroBiasY = sumGyroY / 200.0; // Calculate the "drift" at rest
  
  Serial.println("Ready!");
  lastMicros = micros();
}

void Gyro::update(){
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);

  unsigned long now = micros();
  float dt = (now - lastMicros) / 1000000.0;
  lastMicros = now;

  // 1. Get Tilt from Accelerometer (The "Reference")
  // Using Y and Z because most robots mount the chip vertically
  float accelAngle = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / M_PI;

  // 2. Get Rotation from Gyro (The "Action")
  // Subtract the bias we found during calibration
  float gyroRate = (g.gyro.x * 180.0 / M_PI) - (gyroBiasY * 180.0 / M_PI);

  // 3. Complementary Filter (The "Robot Brain")
  // This is exactly how self-balancing bots stay upright
  angleY = ALPHA * (angleY + gyroRate * dt) + (1.0 - ALPHA) * accelAngle;


}

float Gyro::getY(){
  return angleY;
}