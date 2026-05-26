
#include "Gyro.h"
#include "Arduino.h"
#include <MPU6050_light.h>
#include <Wire.h>

MPU6050 mpu(Wire);

Gyro::Gyro(){

}
void Gyro::init(){
  Wire.begin();
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){ } // stop everything if could not connect to MPU6050
  
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  // 1. Ignore shaking (DLPF)
  mpu.setAccOffsets(0.11,0.02,0.12);
  mpu.setGyroOffsets(-2.59,-1.71,-0.11);
  mpu.writeData(0x1a, 0x03); 
  
  // 2. Trust Gyro more to stop jumpy angles
  mpu.setFilterGyroCoef(0.995); 

  // 3. Calibrate Gyro, but NOT Accel (allows starting on side)
  delay(4000);
  mpu.calcOffsets(false, false);
  Serial.println("Done initializing imu!\n");
  Serial.print("GyroX Offset: "); Serial.println(mpu.getGyroXoffset());
  Serial.print("GyroY Offset: "); Serial.println(mpu.getGyroYoffset());
  Serial.print("GyroZ Offset: "); Serial.println(mpu.getGyroZoffset());


}
void Gyro::CalibrateGyro(){
  mpu.calcGyroOffsets();
}
void Gyro::update(){
  mpu.update();
}

float Gyro::getPitch(){
  return mpu.getAngleX();


}
float Gyro::getYaw(){
  float angle = mpu.getAngleZ();
    // fmod handles the division and returns the remainder for floats
    float wrappedAngle = fmod(angle, 360.0); 
    
    // Ensure the result is always positive (0 to 359.99)
    if (wrappedAngle < 0) {
        wrappedAngle += 360.0;
    }
    return wrappedAngle;
} 