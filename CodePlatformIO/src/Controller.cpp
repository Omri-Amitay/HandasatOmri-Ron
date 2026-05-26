#include "Controller.h"
#include "Arduino.h"
#include <PS4Controller.h>

Controller::Controller(){
}
void Controller::init(){
    PS4.begin("90:89:5F:28:43:8B");
}


Controller::SignalVector Controller::getSignalVector(){
    if(!PS4.isConnected()){
        return {0, 180};
    }

    return {hypot(PS4.RStickX(), PS4.RStickY()), atan2(PS4.RStickY(), PS4.RStickX())};

}
float Controller::convertSignal(int8_t signal){
  return map(signal, -128, 127, -255, 255);
}

bool Controller::clickedA(){
    // Serial.print(PS4.Cross());
    return PS4.Cross();
}