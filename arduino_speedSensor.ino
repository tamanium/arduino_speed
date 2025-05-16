#include <TinyWireM.h>
#include <TinyOzOLED.h>
#include <PinChangeInterrupt.h>

// 速度パルス検知ピン
#define pulseInputPin 3
// パルス出力ピン
#define pulseOutputPin 4
// 
int HALF_SECOND = 527;
int FREQ_DURATION = HALF_SECOND * 20;
long counter = 0;

int freqArr[] = {
  10, 50,
  100,150,
  200, 250,
  300, 350,
  400, 450,
  500, 550,
  600, 650,
  700, 750,
  800, 850,
  900, 950,
  1000,1050,
  1100,1150,
  1200,1250
};

int freqIndex = -1;

void setup(){
  OzOled.init();
  OzOled.setCursorXY(1,1);
  OzOled.printString("monitor", 1, 1);
  OzOled.printString("     km/h", 1, 3);
  OzOled.printString("     Hz", 1, 4);
  OzOled.printString("     Hz", 1, 5);
  tone(pulseOutputPin, freqArr[freqIndex]);
  attachPCINT(digitalPinToPCINT(pulseInputPin), interruption, CHANGE);
}

// 前回総パルス数
long beforeCounter = 0;
// 前回単位時間あたりパルス数
long beforePulseNum = 0;
// 周波数出力時間
long freqOutputTime = 0;
// 周波数変更時間
long freqTime = 0;

void loop(){
  long time = millis();

  // 周波数変更
  if(freqTime <= time){
    int indexMax = sizeof(freqArr)/sizeof(int);
    freqIndex = (++freqIndex)%indexMax;
    tone(pulseOutputPin, freqArr[freqIndex]);
    myPrintLong(freqArr[freqIndex], 1, 5);
    freqTime += FREQ_DURATION;
  }
  
  // 0.5秒ごと周波数算出出力
  if(freqOutputTime <= time){
    long pulseNum = counter;
    long freq = (pulseNum - beforePulseNum) * 2;
    if(0 < freq){
      myPrintLong(long(freq/10), 1, 3);
      myPrintLong(freq, 1, 4);

    }
    beforePulseNum = pulseNum;
    freqOutputTime += HALF_SECOND;
  }
}

// 割り込み処理（カウント）
void interruption(){
  if(digitalRead(pulseInputPin) == HIGH){
    counter = (++counter)%100000;
  }
}

// long変数表示処理
void myPrintLong(long value, int x, int y){
  if(10000 <= value){
    value = value%10000;
  }
  OzOled.setCursorXY(x,y);
  for(int d=1000; 0<d; d/=10){
    if(value/d == 0){
      OzOled.printChar(' ');
    }
    else{
      OzOled.printNumber(value);
      break;
    }
  }
}