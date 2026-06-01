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

double Setpoint = 0, Output = 0, Input = 0;
double Kp = 7.5, Ki = 250, Kd = 0.45;


double setpointYaw = 0, outputYaw = 0, inputYaw = 0;
double KpYaw = 1, KiYaw = 1, KdYaw = 0;

double setpointLeveling = 0, inputLeveling = 0; //input is motorPower, output is the setpoint of the position controller. setpoint is the target rpm.
double KpLeveling = 0.09, KiLeveling = 0, KdLeveling = 0.004;

PID positionController(&Input, &Output, &Setpoint, Kp, Ki, Kd, REVERSE);
PID rotationalController(&inputYaw, &outputYaw, &setpointYaw, KpYaw, KiYaw, KdYaw, DIRECT);

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
double yawOutputLimit = 50;
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
  // velocityTable.addField("Setpoint", motorPID[5]);
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

  levelingController.SetOutputLimits(-4, 4);
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
  // website.update();
  Serial.print(" Hz: ");
  Serial.print(loopTime());
  // Serial.print(" Millis: ");
  // Serial.print(millis());
  // Serial.print(" PS4 Angle: ");
  // Serial.print((controller.getSignalVector().angle + PI)*57.2958);
  // Serial.print(" PS4 Magnitude: ");
  // Serial.print(controller.getSignalVector().magnitude);
  // Serial.print(" Yaw: ");
  // Serial.print(inputYaw);
  // Serial.print(" Output: ");
  // Serial.print(Output);

  Serial.print(" Temp: ");
  Serial.print(gyro.getTemp());
  // leftMotor.print();
  rightMotor.print();
  Serial.println();

  if (positionTable.fieldChanged())
  {
    positionController.SetTunings(Kp, Ki, Kd);
    positionController.SetSampleTime(sampleTime);
    if(positionTable.getFieldByName("ResetGyro")->getValue() == 1){
      gyro.CalibrateGyro();
    }
  }

  if(rotationTable.fieldChanged()){
    rotationalController.SetTunings(KpYaw, KiYaw, KdYaw);
    rotationalController.SetSampleTime(yawSampleTime);
    rotationalController.SetOutputLimits(-yawOutputLimit, yawOutputLimit);
  }
  if(levelingTable.fieldChanged()){
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
  


  Input = gyro.getPitch();
  // inputYaw = gyro.getYaw();
  double motorRPM = (rightMotor.getCalculatedRPM() + leftMotor.getCalculatedRPM()) / 2;
  // inputLeveling = Utils::inRange(motorRPM, 0, 2.5) ? motorRPM * 0.1 : motorRPM;
  rightMotor.update();
  leftMotor.update();
  gyro.update();

  
}

unsigned long lastUpdateOutput = 0; 


float previousOutput = 0;
void loop()
{
  
  if (millis() - lastUpdateSlow > 100)
  {
    // slowUpdates();
  }
  else
  {
    loopTime();
  }
  if(millis() - lastUpdateFast > 5){
    fastUpdates();
  }

  if (true)
  { 
    positionController.Compute();
    if(!Utils::inRange(Output, previousOutput, 0.1)){

      unsigned long now = micros();
      unsigned long runTime = now - lastUpdateOutput;
      lastUpdateOutput = now;
      outputLoopHz = runTime == 0 ? 0 : 1000000.0 / runTime;

      Serial.print(" Previous Output: ");
      Serial.print(previousOutput);
      Serial.print(" Output: ");
      Serial.print(Output);
      Serial.print(" oHZ: ");
      Serial.println(outputLoopHz);
      previousOutput = Output;

      // rotationalController.Compute();
      // levelingController.Compute();
      rightMotor.setPower(abs(Output - outputYaw), Utils::sign(Output - outputYaw));
      leftMotor.setPower(abs(Output + outputYaw), Utils::sign(Output + outputYaw));
    }
    // digitalWrite(statusLed, HIGH);

    // Serial.print(" Output: ");
    // Serial.print(Output);
    // Serial.print( " Pitch ");
    // Serial.println(Input);
   
  }

  if(controller.clickedA()){
    ESP.restart();
  }
  // setpointLeveling = controller.getSignalVector().magnitude;
  // setpointYaw = controller.getSignalVector().angle;

  
}




