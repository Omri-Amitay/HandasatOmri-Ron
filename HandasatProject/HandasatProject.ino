#include "Motor.h"




Motor leftMotor(18,12,25, false);
Motor rightMotor(14,13,26, true);

void setup(){
  leftMotor.init();
  rightMotor.init();
  Serial.begin(9600);
}
void loop(){
  leftMotor.update();
  rightMotor.update();
  rightMotor.setPower(255 , 1);

  Serial.println(rightMotor.getCalculatedRPM());

}