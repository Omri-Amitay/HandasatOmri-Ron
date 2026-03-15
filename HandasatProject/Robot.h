#ifndef Robot_h
#define Robot_h

#include "Arduino.h"
#include "Motor.h"

class Robot{
  public: 
    Robot();
    void init();
    
  private:
  
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