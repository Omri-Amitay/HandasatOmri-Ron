#include "Motor.h"




Motor leftMotor(1,2,3, false);
Motor rightMotor(1,2,3, false);

void setup(){
  leftMotor.init();
  rightMotor.init();
}
void loop(){
  leftMotor.update();
  rightMotor.update();


}