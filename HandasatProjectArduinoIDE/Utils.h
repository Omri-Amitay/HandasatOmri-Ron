#ifndef Utils_h
#define Utils_h

#include <Arduino.h>

class Utils {
public:
    static int sign(float val);
    static float clamp(float val,float min, float max);
    static bool recievedNumber(double &outValue);
    static bool recieve2Numbers(double &outA, double &outB);
    static bool inRange(float val, float target, float tolerance);
};

#endif