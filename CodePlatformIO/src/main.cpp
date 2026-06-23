#include "Arduino.h"

#include "Motor.h"
#include "Gyro.h"
#include "Utils.h"
#include "Website.h"
#include "config.h"
#include <PID_v1.h>

#include "Controller.h"

Controller controller;

Motor leftMotor(" left motor ", 18, 12, 25, false);
Motor rightMotor(" right motor ", 14, 13, 26, true);

Gyro gyro;
int statusLed = 2;

double Setpoint = 1, Output = 0, Input = 0;
double Kp = 15, Ki = 200, Kd = 0.12;


double setpointYaw = 0, outputYaw = 0, inputYaw = 0;
double KpYaw = 1, KiYaw = 0, KdYaw = 0;

double setpointLeveling = 0, inputLeveling = 0; //input is motorPower, output is the setpoint of the position controller. setpoint is the target rpm.
double KpLeveling = 0.13, KiLeveling = 0.05, KdLeveling = 0.001;

PID positionController(&Input, &Output, &Setpoint, Kp, Ki, Kd, REVERSE);

double yawDisTarget = 0;
PID rotationalController(&inputYaw, &outputYaw, &yawDisTarget, KpYaw, KiYaw, KdYaw, REVERSE);

PID levelingController(&inputLeveling, &Setpoint, &setpointLeveling, KpLeveling, KiLeveling, KdLeveling, DIRECT);

TunablesManager manager;
Website website(&manager);

unsigned long lastLoop = 0;
unsigned long lastInRange = 0;

bool hasStarted = false;

TunableGroup positionTable("position");
TunableGroup rotationTable("rotation");
TunableGroup velocityTable("motorVelocity");
TunableGroup levelingTable("leveling");

double **motorPID;
double motorPower;
double sampleTime = 10;
double yawSampleTime = 30;
double yawOutputLimit = 70;
double resetGyro = 0;
void setTunables()
{
  levelingTable.addField("Kp", &KpLeveling);
  levelingTable.addField("Ki", &KiLeveling);
  levelingTable.addField("Kd", &KdLeveling);
  levelingTable.addField("Setpoint", &setpointLeveling);

  rotationTable.addField("Kp", &KpYaw);
  rotationTable.addField("Ki", &KiYaw);
  rotationTable.addField("Kd", &KdYaw);
  rotationTable.addField("Setpoint", &setpointYaw);
  rotationTable.addField("SampleTime", &yawSampleTime);
  rotationTable.addField("OutputLimit", &yawOutputLimit);

  positionTable.addField("Kp", &Kp);
  positionTable.addField("Ki", &Ki);
  positionTable.addField("Kd", &Kd);
  
  
  positionTable.addField("Setpoint", &Setpoint);
  positionTable.addField("SampleTime", &sampleTime);
  positionTable.addField("ResetGyro", &resetGyro);

  motorPID = rightMotor.getPIDValues();
  velocityTable.addField("Kp", motorPID[0]);
  velocityTable.addField("Ki", motorPID[1]);
  velocityTable.addField("Kd", motorPID[2]);
  velocityTable.addField("Ks", motorPID[3]);
  velocityTable.addField("Kv", motorPID[4]);
  velocityTable.addField("Setpoint", motorPID[5]);
  velocityTable.addField("MotorPower", &motorPower);

  manager.addGroup(&levelingTable);
  manager.addGroup(&rotationTable);
  manager.addGroup(&positionTable);
  manager.addGroup(&velocityTable);

}

RTC_DATA_ATTR static bool hasSoftwareResetOccurred = false;
void setup()
{

  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);
  
  
  Serial.begin(115200);

  
  esp_reset_reason_t resetReason = esp_reset_reason();
  Serial.print("Hardware Boot Reason: ");
  Serial.println(resetReason);

  // If the robot was turned on via the physical battery switch (Power-on reset)
  // if (resetReason == ESP_RST_POWERON || resetReason == ESP_RST_EXT) {
  //   Serial.println("Cold boot detected! Waiting for power rails to stabilize, then restarting...");
  //   delay(3000);   // Give the battery voltage a full 2 seconds to level out
  //   digitalWrite(statusLed, LOW);
  //   delay(6000);
  //   digitalWrite(statusLed, HIGH);
  //   delay(6000);
  //   ESP.restart(); // Trigger the one-time software reset
  // }
  for (int i = 0; i < 6; i++) {
    digitalWrite(statusLed, LOW);
    delay(50);
    digitalWrite(statusLed, HIGH);
    delay(50);
  }
  
  leftMotor.init();
  rightMotor.init();
  
  rightMotor.setPower(0,1);
  leftMotor.setPower(0,1);

  controller.init();
  
  
  setTunables();
  website.init();

  
  gyro.init();
  positionController.SetOutputLimits(-255, 255);
  positionController.SetMode(AUTOMATIC);
  positionController.SetSampleTime(10);

  rotationalController.SetOutputLimits(-yawOutputLimit, yawOutputLimit);
  rotationalController.SetMode(AUTOMATIC);
  rotationalController.SetSampleTime(yawSampleTime);

  levelingController.SetOutputLimits(-6, 6);
  levelingController.SetMode(AUTOMATIC);
  levelingController.SetSampleTime(40);

  Serial.println("Robot Initialized!");
  
  digitalWrite(statusLed, LOW);
}

float loopTime()
{
  unsigned long now = micros();
  unsigned long runTime = now - lastLoop;
  lastLoop = now;
  return runTime == 0 ? 0 : 1000000.0 / runTime;
}

bool inRange = false;

unsigned long lastUpdateSlow = 0;

float outputLoopHz = 0;

void slowUpdates()
{
  lastUpdateSlow = millis();
  website.update();
  // Serial.print(" Hz: ");
  // Serial.print(loopTime());
  // // Serial.print(" Millis: ");
  // // Serial.print(millis());
  // // Serial.print(" PS4 Angle: ");
  // // Serial.print((controller.getSignalVector().angle + PI)*57.2958);
  // // Serial.print(" PS4 Magnitude: ");
  // // Serial.print(controller.getSignalVector().magnitude);
  // Serial.print(" Yaw: ");
  // Serial.print(inputYaw);
  Serial.print(" Setpoint: ");
  Serial.print(Setpoint);
  Serial.print(" Pitch: ");
  Serial.print(gyro.getPitch());
  Serial.println();
  // // Serial.print(" Output: ");
  // // Serial.print(Output);

  // Serial.print(" Temp: ");
  // Serial.print(gyro.getTemp());

  // Serial.print(" yaw ");
  // Serial.print( gyro.getYaw());
  // Serial.print(" Direction ");
  // Serial.print( Utils::calculateShortestPath(gyro.getYaw(), setpointYaw));
  // Serial.print(" Setpoint: ");
  // Serial.print( setpointYaw);

  // Serial.print(" rotationSetpoint: ");
  // Serial.print( setpointLeveling );
  // Serial.print(" magnitude ");
  // Serial.print( controller.getSignalVector().magnitude );
  // leftMotor.print();
  // rightMotor.print();
  // Serial.println();

  if (positionTable.fieldChanged() != -1)
  {
    positionController.SetTunings(Kp, Ki, Kd);
    positionController.SetSampleTime(sampleTime);
    if(positionTable.getFieldByName("ResetGyro")->getValue() == 1){
      gyro.CalibrateGyro();
    }
    Serial.print("!!!!!!!!");
  }
  int rotationControllerField = rotationTable.fieldChanged();
  if(rotationControllerField != -1)
  {
    rotationalController.SetTunings(KpYaw, KiYaw, KdYaw);
    rotationalController.SetSampleTime(yawSampleTime);
    rotationalController.SetOutputLimits(-yawOutputLimit, yawOutputLimit);
    Serial.print(" RC KP: " + String( rotationTable.getFieldByIndex(rotationControllerField)->getValue()));
    Serial.print(rotationalController.GetKd());
  }
  if(levelingTable.fieldChanged() != -1){
    levelingController.SetTunings(KpLeveling, KiLeveling, KdLeveling);
  }
  int velFieldChange = velocityTable.fieldChanged();
  if (velFieldChange != -1)
  {
    Serial.print("Vel FieldChange: ");
    Serial.print(velocityTable.getFieldByIndex(velFieldChange)->getValue());
    
    
    if (motorPower != 0)
    {
      rightMotor.setVelControl(false);
      leftMotor.setVelControl(false);
      // leftMotor.setPower(abs(motorPower), Utils::sign(motorPower));
      // rightMotor.setPower(abs(motorPower), Utils::sign(motorPower));
    }
    else
    {
      rightMotor.setVelControl(true);
      leftMotor.setVelControl(true);
    }

    rightMotor.updatePID(motorPID);
    leftMotor.updatePID(motorPID);
  }
}


unsigned long lastUpdateFast = 0;


void fastUpdates(){
  
  if (!Utils::inRange(Input, Setpoint, 70))
  {
    // hasStarted = false;
  }
  
  
  inRange = Utils::inRange(Input, Setpoint, 1);

  // {
  //   if (!hasStarted && lastInRange != 0)
  //   {
  //     positionController.SetMode(AUTOMATIC);
  //     // rightMotor.setVelControl(true);
  //     // leftMotor.setVelControl(true);
  //     hasStarted = true;
  //   }
  //   else
  //   {
  //     lastInRange = millis();
  //   }
  // }
    // if(controller.clickedA() || motorPower != 0){
    //   hasStarted = true;
    // }
  




  
}

unsigned long lastUpdateOutput = 0; 


float previousOutput = 0;

bool flipDirection(float gyro, float wantedAngle){
  return abs(Utils::calculateShortestPath(gyro, wantedAngle)) < abs(Utils::calculateShortestPath(gyro + 180, wantedAngle));
}

void loop()
{
  
  if (millis() - lastUpdateSlow > 100)
  {
    slowUpdates();
  }
  else
  {
    loopTime();
  }
  if(millis() - lastUpdateFast > 5){
    fastUpdates();
  }


  gyro.update();

  if (true)
  { 
    Input = gyro.getPitch();
    positionController.Compute();
    if(!Utils::inRange(Output, previousOutput, 0.01)){
      unsigned long now = micros();
      unsigned long runTime = now - lastUpdateOutput;
      lastUpdateOutput = now;
      outputLoopHz = runTime == 0 ? 0 : 1000000.0 / runTime;
      
      previousOutput = Output;
      rotationalController.Compute();
      levelingController.Compute();
      
      rightMotor.setPower(abs(Output - outputYaw), Utils::sign(Output - outputYaw));
      leftMotor.setPower(abs(Output + outputYaw), Utils::sign(Output + outputYaw));
      
      inputYaw = Utils::calculateShortestPath(gyro.getYaw(), setpointYaw);
      double motorRPM = (rightMotor.getCalculatedRPM() + leftMotor.getCalculatedRPM()) / 2;
      inputLeveling = Utils::inRange(motorRPM, 0, 4) ? motorRPM : motorRPM;
      rightMotor.update();
      leftMotor.update();
      
      float magnitude = controller.getSignalVector().magnitude;

      
      float gyroAngle = gyro.getYaw();
      float controllerAngle = controller.getSignalVector().angle;
      bool shouldFlipDirection = flipDirection(gyroAngle, controllerAngle);

      if(!shouldFlipDirection){
        setpointYaw = fmod(controllerAngle + 180.0, 360.0);
      }else{
        setpointYaw = controllerAngle;
      }
      if(Utils::inRange(controllerAngle, 90, 0.001)){
        setpointYaw = gyroAngle;
      }

      setpointLeveling = shouldFlipDirection ? magnitude : magnitude * -1;
      // Serial.print("HZ");
      // Serial.print(outputLoopHz);
      // Serial.println();

      // Serial.print(" Shouldflip: ");
      // Serial.print(shouldFlipDirection);
      // Serial.print(" CA ");
      // Serial.print(controllerAngle);
      // Serial.print(" gyroAngle ");
      // Serial.println(gyroAngle);
      // Serial.print(" Distance ");
      // Serial.print(inputYaw);
      // Serial.print(" yawSetpoint: ");
      // Serial.print(setpointYaw);
    }
   
  }

  if(controller.clickedA() || motorPower != 0){
    ESP.restart();
  }
  if(controller.clickedB()){
    gyro.resetYaw();
  }

  
}






