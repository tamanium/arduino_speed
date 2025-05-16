#include <TinyWireM.h>
#include <TinyOzOLED.h>
#include <PinChangeInterrupt.h>

// 速度パルス検知ピン
#define pulsePin 4

int TIME_DURATION = 1000;

void setup(){
  long start = millis();
  OzOled.init();
  //OzOled.clearDisplay();
  long duration = millis() - start;
  OzOled.setCursorXY(1,1);
  OzOled.printString("display on");
  OzOled.setCursorXY(1,3);
  OzOled.printString("---");
  attachPCINT(digitalPinToPCINT(pulsePin), interruption, CHANGE);
}

long clockTime = 0;
long counter = 0;
void loop(){
  long time = millis();
  if(clockTime < time){
    OzOled.printNumber(counter++,1,2);
    clockTime += TIME_DURATION;
  }
}

void interruption(){

}