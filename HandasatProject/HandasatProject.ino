#include "Motor.h"
#include "Gyro.h"
#include "Utils.h"
#include <PID_v1.h>

Motor leftMotor(" left motor ",18,12,25, true);
Motor rightMotor(" right motor ",14,13,26, false);

Gyro gyro;
int statusLed = 4;

double Setpoint = 0, Output = 0, Input = 0;
double Kp = 10,Ki = 0,Kd = 0;
PID positionController(&Input, &Output, &Setpoint, Kp, Ki, Kd, REVERSE);


unsigned long lastLoop = 0;
unsigned long lastInRange = 0;

bool hasStarted = false;


void setup(){
  Serial.begin(115200);

  leftMotor.init();
  rightMotor.init();
  rightMotor.setVelocity(0);
  leftMotor.setVelocity(0);

  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, LOW);
  gyro.init();

  positionController.SetOutputLimits(-40, 40);
  positionController.SetSampleTime(60);
  positionController.SetMode(MANUAL);
  
  Serial.println("Robot Initialized!");
  
}
void loop(){

  double newKd = 0;
  if(Utils::recievedNumber(newKd)){
    Kd = newKd;
    rightMotor.setTunableNumber(Kd, 0);
    leftMotor.setTunableNumber(Kd, 0);
  }
  if(hasStarted){
    digitalWrite(statusLed, HIGH);
    positionController.Compute();
    rightMotor.setVelocity(Output);
    leftMotor.setVelocity(Output);
  }else{
    rightMotor.setVelocity(0);
    leftMotor.setVelocity(0); 
  }
  if(!Utils::inRange(Input, Setpoint, 30)){
    hasStarted = false;
  }
  Input = gyro.getY();
  bool inRange = Utils::inRange(Input, Setpoint, 5);
  if( inRange){
    if(!hasStarted && lastInRange != 0){
      positionController.SetMode(AUTOMATIC);
      hasStarted = true;
    }else{
      lastInRange = millis();
    }
  }

  // Serial.print(" inRnage: ");
  // Serial.print( inRange );
  // Serial.print(" Loop Frequency: ");
  // Serial.print(loopTime());

  Input = gyro.getY();
  
  rightMotor.update();
  leftMotor.update();
  gyro.update();
  Serial.println();
}

float loopTime(){
  unsigned long runTime = millis() - lastLoop;
  lastLoop = millis();
  return runTime == 0 ? 0 : 1000/runTime;
}