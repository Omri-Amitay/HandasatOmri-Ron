#include "Motor.h"
#include "Gyro.h"
#include "Utils.h"
#include "Website.h"
#include "config.h"
#include <PID_v1.h>





Motor leftMotor(" left motor ",18,12,25, true);
Motor rightMotor(" right motor ",14,13,26, false);

Gyro gyro;
int statusLed = 4;

double Setpoint = 0, Output = 0, Input = 0;
double Kp = 10,Ki = 0,Kd = 0;
PID positionController(&Input, &Output, &Setpoint, Kp, Ki, Kd, REVERSE);

TunablesManager manager;
Website website(&manager);

unsigned long lastLoop = 0;
unsigned long lastInRange = 0;

bool hasStarted = false;

TunableGroup positionTable("position");
TunableGroup velocityTable("motorVelocity");
//TunableGroup motorTable("motor");


double** motorPID;
double motorPower;
void setTunables(){
  positionTable.addField("Kp", &Kp);
  positionTable.addField("Ki", &Ki);
  positionTable.addField("Kd", &Kd);
  positionTable.addField("Setpoint", &Setpoint);

  motorPID = rightMotor.getPIDValues();
  velocityTable.addField("Kp",motorPID[0]);
  velocityTable.addField("Ki",motorPID[1]);
  velocityTable.addField("Kd",motorPID[2]);
  velocityTable.addField("Ks", motorPID[3]);
  velocityTable.addField("Kv", motorPID[4]);
  //velocityTable.addField("Setpoint", motorPID[5]);
  velocityTable.addField("MotorPower", &motorPower);

  manager.addGroup(&positionTable);
  manager.addGroup(&velocityTable);
}

void setup(){
  Serial.begin(115200);
  leftMotor.init();
  rightMotor.init();
  rightMotor.setVelocity(0);
  leftMotor.setVelocity(0);
  setTunables();
  website.init();

  



  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, LOW);
  gyro.init();

  positionController.SetOutputLimits(-40, 40);
  positionController.SetSampleTime(60);
  positionController.SetMode(AUTOMATIC);
  
  Serial.println("Robot Initialized!");
  
}
void loop(){
  website.update();

  if(positionTable.fieldChanged()){
    positionController.SetTunings(Kp,Ki,Kd);
    
  }
  int velFieldChange = velocityTable.fieldChanged();
  if(velFieldChange != -1){
    Serial.print("Vel FieldChange: ");
    Serial.print(velocityTable.getFieldByIndex(velFieldChange)->getValue());
    if(motorPower != 0){
      rightMotor.setVelControl(false);
      leftMotor.setVelControl(false);
    }else{
      rightMotor.setVelControl(true);
      leftMotor.setVelControl(true);
    }
    rightMotor.updatePID(motorPID);
    rightMotor.setPower(abs(motorPower), Utils::sign(motorPower));
    leftMotor.updatePID(motorPID);
    leftMotor.setPower(abs(motorPower), Utils::sign(motorPower));
  }
  Serial.println();
  double newKd = 0;
  
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
      rightMotor.setVelControl(true);
      leftMotor.setVelControl(true);
      hasStarted = true;
    }else{
      lastInRange = millis();
    }
  }

  Serial.print(" inRnage: ");
  Serial.print( inRange );
  // Serial.print(" Loop Frequency: ");
  // Serial.print(loopTime());

  Input = gyro.getY();
  
  rightMotor.update();
  leftMotor.update();
  gyro.update();

}

float loopTime(){
  unsigned long runTime = millis() - lastLoop;
  lastLoop = millis();
  return runTime == 0 ? 0 : 1000/runTime;
}