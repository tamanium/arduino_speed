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
  OzOled.printString("monitor", 1, 0);
  OzOled.printString("     km/h", 1, 1);
  OzOled.printString("     Hz in1", 1, 2);
  OzOled.printString("     Hz in", 1, 3);
  OzOled.printString("     Hz out", 1, 4);
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
long pulseSpans = 0;

void loop(){
  long time = millis();

  // 周波数変更
  if(freqTime <= time){
    int indexMax = sizeof(freqArr)/sizeof(int);
    freqIndex = (++freqIndex)%indexMax;
    tone(pulseOutputPin, freqArr[freqIndex]);
    myPrintLong(freqArr[freqIndex], 1, 4);
    freqTime += FREQ_DURATION;
  }
  
  // 0.5秒ごと周波数算出出力
  if(freqOutputTime <= time){
    // 割り込み中断
		noInterrupts();
    long spansTotal = pulseSpans;
    long pulseNum = counter;
    pulseSpans = 0;
    // 割り込み再開
		interrupts();
    double freqDouble = (pulseNum - beforePulseNum) * 1000000 / spansTotal;
    long freq1 = (long)freqDouble;
    long freq = (pulseNum - beforePulseNum) * 2;
    if(0 < freq){
      myPrintLong(long(freq/10), 1, 1);
      myPrintLong(freq1, 1, 2);
      myPrintLong(freq, 1, 3);
    }
    beforePulseNum = pulseNum;
    freqOutputTime += HALF_SECOND;
  }
}

volatile unsigned long beforeTime = 0;
// 割り込み処理（カウント）
void interruption(){
  if(digitalRead(pulseInputPin) == HIGH){
    counter = (++counter)%100000;
    unsigned long time = micros();
    pulseSpans += time - beforeTime;
    beforeTime = time;
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