int FG = 12;
int powerPin = 3;
int directionPin = 7;
int power = 255;
const int totalReadings = 10;
long smoothingArray[totalReadings];
int readIndex = 0;
unsigned long total = 0;

int colorSensor = 11;
bool valueSmoothed = false;
void setup() {  // put your setup code here, to run once:
  for (int i = 0; i < totalReadings; i++) { smoothingArray[i] = 0; }
  pinMode(colorSensor, INPUT);
  pinMode(FG, INPUT_PULLUP);
  pinMode(powerPin, OUTPUT);
  pinMode(directionPin, OUTPUT);
  digitalWrite(directionPin, HIGH);
  Serial.begin(115200);
 
}
long lastUpdate = 0;
int largestNum = 0;
int fastestPulse = 420;
long slowestPulse = 18000;
float floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

unsigned long lastDetection = 0;
bool colorSensorState = false;
float getRPM(){
  float rpm = -1;
  if(digitalRead(colorSensor)){
    
    if(lastDetection + 600 < millis()){
      
      unsigned long diff = millis() - lastDetection;
      lastDetection = millis();
      
      rpm = 60.0 /(diff/1000.0);
      


    }else{
      lastDetection = millis();
    }
  }
  
  return rpm;
}

int deletedSmoothingNum = 2; //cycle over all of SmoothingArray deletedSmoothingNum before sending valueSmoothed = true

float getMotorPulse(int FG){

  total -= smoothingArray[readIndex];

  unsigned long highPulse = pulseIn(FG, HIGH, slowestPulse);

  if(highPulse < fastestPulse){
    highPulse = slowestPulse;
  }

  smoothingArray[readIndex] = highPulse;

  /*Serial.print(" AddingValue: " + String(smoothingArray[readIndex]));*/

  total += smoothingArray[readIndex];
  float average = total / totalReadings;

 /* Serial.print("Average Sample: " + String(average) + " Current Index: " + String(readIndex) + " Total: " + String(total) + " ");
  Serial.print(" [");
  for (int i = 0; i < totalReadings; i++) { Serial.print(String(smoothingArray[i]) + ", "); }
  Serial.print("]");
  */
  readIndex = (readIndex + 1) % totalReadings;
 
  if(readIndex == 0){
    deletedSmoothingNum--;
    
    if(deletedSmoothingNum == 0){
      valueSmoothed = true;
      deletedSmoothingNum = 1;//stupid way to prevent deletedSmoothing into going negative
    }
  }
  
  return average;
}

unsigned long passedSinceLastSignal = 0;
int NumOfRotations = 0;
void loop() {
  /*  long highInput = pulseIn(FG, HIGH, 1000000);
  long lowInput = pulseIn(FG, LOW, 1000000);
  long period = highInput + lowInput;

  Serial.println("High Input: " + String(highInput) + " Low Input: " + String(lowInput) + " Period" + String(period) + " MaxTimeout: " + String(maxTimeout));
  if (highInput < maxTimeout) { maxTimeout = highInput; }
  if (lowInput < maxTimeout) { maxTimeout = lowInput; }
  
  
  if (lastUpdate + 100 < millis()) {
    power++;
    lastUpdate = millis();
  }
  

  // max value 17700 min value 420 power = 100;
  if (power > 255) {
    power = 0;
  }*/
  analogWrite(powerPin, power);  //getMotorSpeed(FG); Serial.println(getMotorSpeed(FG)); 
  float rpm = getRPM();
  
  if(rpm != -1){
    NumOfRotations++;
    Serial.println(NumOfRotations);
  }
  if(rpm != -1 && valueSmoothed && passedSinceLastSignal + 1 < millis() && NumOfRotations >= 3){
    NumOfRotations = 0;
    passedSinceLastSignal = millis();
    valueSmoothed = false;
    
    
   
    Serial.println(String(rpm) + "," + String(getMotorPulse(FG)));
    Serial.println(power);
    power -= 10;
  
  }
  //Serial.println(!digitalRead(colorSensor));
  getMotorPulse(FG);
  if(power < 16){
    Serial.println("done" + String(power));
  }
}