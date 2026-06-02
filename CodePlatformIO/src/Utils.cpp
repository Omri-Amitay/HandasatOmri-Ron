#include "Utils.h"

int Utils::sign(float val){
  return val == 0 ? 0 : (val/abs(val) > 0) ? 1 : -1;
}

float Utils::clamp(float val, float min, float max){
  if(val > max){
    return max;
  }else if(val < min){
    return min;
  }else{
    return val;
  }
}
bool Utils::recievedNumber(double &outValue) {
  static String buffer = "";

  if (Serial.available() <= 0) {
    return false;
  }

  char c = Serial.read();

  if (c == '\r') {
    return false;
  }

  if (c == '\n') {
    buffer.trim();

    if (buffer.length() == 0) {
      buffer = "";
      return false;
    }

    char temp[32];
    buffer.toCharArray(temp, sizeof(temp));

    char* endPtr;
    double parsed = strtod(temp, &endPtr);

    buffer = "";

    if (*endPtr == '\0') {
      outValue = parsed;
      return true;
    }

    return false;
  }

  buffer += c;
  return false;
}

bool Utils::recieve2Numbers(double &outA, double &outB) {
  static String buffer = "";

  if (Serial.available() <= 0) {
    return false;
  }

  char c = Serial.read();

  if (c == '\r') {
    return false;
  }

  if (c == '\n') {
    buffer.trim();

    if (buffer.length() == 0) {
      buffer = "";
      return false;
    }

    int commaIndex = buffer.indexOf(',');
    if (commaIndex == -1) {
      buffer = "";
      return false;
    }

    String firstPart = buffer.substring(0, commaIndex);
    String secondPart = buffer.substring(commaIndex + 1);

    firstPart.trim();
    secondPart.trim();

    char temp1[32];
    char temp2[32];

    firstPart.toCharArray(temp1, sizeof(temp1));
    secondPart.toCharArray(temp2, sizeof(temp2));

    char *endPtr1;
    char *endPtr2;

    double val1 = strtod(temp1, &endPtr1);
    double val2 = strtod(temp2, &endPtr2);

    buffer = "";

    if (*endPtr1 == '\0' && *endPtr2 == '\0') {
      outA = val1;
      outB = val2;
      return true;
    }

    return false;
  }

  buffer += c;
  return false;
}

bool Utils::inRange(float val, float target, float tolerance){
  if(fabs(val-target) < tolerance ){
    return true;
  }
  return false;
}

float Utils::calculateShortestPath(float angle, float wantedAngle){
  return fmod(wantedAngle - angle + 540.0, 360.0) - 180;
}