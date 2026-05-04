// #include "Motor.h"
// #include "Gyro.h"
// #include "Utils.h"
// #include <PID_v1.h>

// Motor leftMotor(" left motor ",18,12,25, true);
// Motor rightMotor(" right motor ",14,13,26, false);

// Gyro gyro;

// PID positionController()


// unsigned long lastLoop = 0;
// unsigned long lastSetpointSwitch = 0;

// void setup(){
//   Serial.begin(115200);
//   leftMotor.init();
//   rightMotor.init();
//   //gyro.init();
//   //rightMotor.setVelocity(0);
//   leftMotor.setVelocity(0);
  
// }
// void loop(){
//   double kp = 0;
//   double ki = 0;
//   double kd = 0;
//   // if(Utils::recieve2Numbers(ki,kd)){
//   //   rightMotor.setTunableNumber(ki,kd);
//   //   leftMotor.setTunableNumber(ki, kd);
//   // }
//   // if(Utils::recievedNumber(setpoint)){
    
//   //   rightMotor.setVelocity(setpoint);
//   //   leftMotor.setVelocity(setpoint);
//   // }
//   // if(lastSetpointSwitch + 3000 < millis()){
//   //   lastSetpointSwitch = millis();
//   //   if(setpoint == 20){
//   //     setpoint = 40;
//   //   }else{
//   //     setpoint = 20;
//   //   }
//   // }

//   // int sign = 32;
//   // Serial.print(Utils::sign(sign));
//   // Serial.print(" :: ");
//   // sign = 0;
//   // Serial.print(Utils::sign(sign));
//   // Serial.print(" :: ");
//   // sign = -20;
  
//   // Serial.print(Utils::sign(sign));
  
//   // Serial.print(" :: ");

//   // Serial.print(Utils::clamp(millis() % 1000, 400, 800));
  
//   // Serial.println();
  
//   // leftMotor.update();
//   rightMotor.update();
//   leftMotor.update();
//   //gyro.update();
//   // rightMotor.setPower(50, 1);
//   // leftMotor.setPower(50, 1);

//   //Serial.print(loopTime());
//   Serial.println();
//   //Serial.println(rightMotor.getCalculatedRPM());

// }

// float loopTime(){
//   unsigned long runTime = millis() - lastLoop;
//   lastLoop = millis();
//   return 1000/runTime;
// }