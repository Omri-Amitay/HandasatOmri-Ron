#include "Arduino.h"

#include "Motor.h"
#include "Gyro.h"
#include "Utils.h"
#include "Website.h"
#include "config.h"
#include <PID_v1.h>

#include <PS4Controller.h>

Motor leftMotor(" left motor ", 18, 12, 25, false);
Motor rightMotor(" right motor ", 14, 13, 26, true);

Gyro gyro;
int statusLed = 4;

double Setpoint = 0, Output = 0, Input = 0;
double Kp = 9, Ki = 240, Kd = 0.26;


double setpointYaw = 0, outputYaw = 0, inputYaw = 0;
double KpYaw = 1, KiYaw = 1, KdYaw = 0;

double setpointLeveling = 0, inputLeveling = 0; //input is motorPower, output is the setpoint of the position controller. setpoint is the target rpm.
double KpLeveling = 1, KiLeveling = 1, KdLeveling = 0;

PID positionController(&Input, &Output, &Setpoint, Kp, Ki, Kd, REVERSE);
PID rotationalController(&inputYaw, &outputYaw, &setpointYaw, KpYaw, KiYaw, KdYaw, DIRECT);

PID levelingController(&inputLeveling, &Setpoint, &setpointLeveling, KpLeveling, KiLeveling, KdLeveling, REVERSE);

TunablesManager manager;
Website website(&manager);

unsigned long lastLoop = 0;
unsigned long lastInRange = 0;

bool hasStarted = false;

TunableGroup positionTable("position");
TunableGroup rotationTable("rotation");
TunableGroup velocityTable("motorVelocity");
TunableGroup levelingTable("leveling");
// TunableGroup motorTable("motor");

double **motorPID;
double motorPower;
double sampleTime = 10;
double yawSampleTime = 30;
double yawOutputLimit = 10;
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

void setup()
{
  Serial.begin(115200);
  leftMotor.init();
  rightMotor.init();
  delay(2000);
  // rightMotor.setVelocity(0);
  // leftMotor.setVelocity(0);
  rightMotor.setPower(0,1);
  leftMotor.setPower(0,1);
  setTunables();
  website.init();

  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, LOW);
  gyro.init();
// if (!PS4.begin("90:89:5F:28:43:8B")) {
//     Serial.println("Fatal Error: PS4 Bluetooth failed to initialize!");
//     while(1); // Stop right here if the hardware failed to start
//   }
  positionController.SetOutputLimits(-255, 255);
  positionController.SetMode(AUTOMATIC);
  positionController.SetSampleTime(sampleTime);

  rotationalController.SetOutputLimits(-yawOutputLimit, yawOutputLimit);
  rotationalController.SetMode(AUTOMATIC);
  rotationalController.SetSampleTime(yawSampleTime);

  levelingController.SetOutputLimits(-2, 2);
  levelingController.SetMode(AUTOMATIC);
  levelingController.SetSampleTime(40);

  Serial.println("Robot Initialized!");
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

void slowUpdates()
{
  // if(PS4.isConnected()){

  //   (millis()/ 1000) % 2 == 0 ? PS4.setLed(255,0,0) : PS4.setLed(0,255,0);
  //   PS4.setRumble(100, 255);
  //   if(PS4.Right()){
  //     setpointYaw = 0;
  //    }
  //   else if(PS4.Left()){
  //     setpointYaw = 90;
  //     Serial.print(" Left ");
  //   }
  //   Serial.print(" PS4 Connected! ");
  // }
  lastUpdateSlow = millis();
  website.update();
  Serial.print(" Hz: ");
  Serial.print(loopTime());
  Serial.print(" iR: ");
  Serial.print(inRange);
  Serial.print(" hS: ");
  Serial.print(hasStarted);
  Serial.print(" Gy: ");
  Serial.print(Input);
  Serial.print(" MO: ");
  Serial.print(Output);
  Serial.print(" MotorWantedPower: ");
  Serial.print(motorPower);
  Serial.print( " levelingInput: ");
  Serial.print(inputLeveling);
  Serial.print(" SetpointPitch: ");
  Serial.print(Setpoint);
  
  
  rightMotor.print();
  leftMotor.print();
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
      leftMotor.setPower(abs(motorPower), Utils::sign(motorPower));
      rightMotor.setPower(abs(motorPower), Utils::sign(motorPower));
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
  
  if (hasStarted)
  {
    digitalWrite(statusLed, HIGH);
    positionController.Compute();
    rotationalController.Compute();
    levelingController.Compute();
    // rightMotor.setVelocity(Output);
    // leftMotor.setVelocity(Output);
    rightMotor.setPower(abs(Output - outputYaw), Utils::sign(Output - outputYaw));
    leftMotor.setPower(abs(Output + outputYaw), Utils::sign(Output + outputYaw));
   
  }
  else
  {
    rightMotor.setPower(0,1);
    leftMotor.setPower(0,1);
  }
  if (!Utils::inRange(Input, Setpoint, 70))
  {
    //hasStarted = false;
  }
  
  
  inRange = Utils::inRange(Input, Setpoint, 1);

  if (inRange)
  {
    if (!hasStarted && lastInRange != 0)
    {
      positionController.SetMode(AUTOMATIC);
      // rightMotor.setVelControl(true);
      // leftMotor.setVelControl(true);
      hasStarted = true;
    }
    else
    {
      lastInRange = millis();
    }
  }


  Input = gyro.getPitch();
  inputYaw = gyro.getYaw();
  inputLeveling = (rightMotor.getCalculatedRPM() + leftMotor.getCalculatedRPM()) / 2;
  rightMotor.update();
  leftMotor.update();
  gyro.update();
}
void loop()
{
  
  if (millis() - lastUpdateSlow > 50)
  {
    slowUpdates();
  }
  else
  {
    loopTime();
  }
  if(millis() - lastUpdateFast > 6){
    fastUpdates();
  }



}
