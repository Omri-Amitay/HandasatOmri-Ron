
#ifndef Controller_h
#define Controller_h

#include <Arduino.h>

class Controller{
  public:

    Controller();

     struct SignalVector{
      float magnitude;
      float angle;
    };


    void init();
    void update();
    SignalVector getSignalVector();
    bool clickedA();
    bool clickedB();
   
  private:
  
    float convertSignal(int8_t signal);
    
};

#endif
