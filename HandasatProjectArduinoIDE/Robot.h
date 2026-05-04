#ifndef Robot_h
#define Robot_h

#include "Arduino.h"
#include "Motor.h"

class Robot{
  public: 
    Robot();
    void init();
    
  private:
  
  // Motor leftMotor(" left motor ",18,12,25, true);
  // Motor rightMotor(" right motor ",14,13,26, false);

  enum RobotStates = {
    NONE, 
    INIT,
    PRE_START,
    STABILIZE
  }
  struct StateInfoStruct {
    String name;
  }
  const StateInfoStruct stateInfo[] = {
    {"None"},
    {"Pre_Start"},
    {"Stabilize"}
  }

  RobotStates currentState = RobotStates.INIT;

}

#endif