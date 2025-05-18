//#define DEBUG_MODE
#define PULSE_OUTPUT_MODE
#ifndef DEBUG_MODE
  #include <Wire.h>
#endif
#ifdef DEBUG_MODE
  #include <TinyWireM.h>
  #include <TinyOzOLED.h>
#endif
#include <PinChangeInterrupt.h>

// 速度パルス検知ピン
#define pulseInputPin 3
// パルス出力ピン
#define pulseOutputPin 4
// I2Cアドレス
#define address 0x55

// 定数
const int HALF_SECOND = 527;
const int FREQ_DURATION = HALF_SECOND * 20;
const int FREQ = 0;
const int SPEED = 1;
// 変数
long counter = 0;
byte buffer[2] = {0, 0};
int regIndex = 0;

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
  #ifdef DEBUG_MODE
    OzOled.init();
    OzOled.printString("monitor", 1, 0);
    OzOled.printString("     km/h", 1, 2);
    OzOled.printString("     Hz in", 1, 3);
  #endif 
  #ifndef DEBUG_MODE
    Wire.begin(address);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
  #endif
  #ifdef PULSE_OUTPUT_MODE
    #ifdef DEBUG_MODE
      OzOled.printString("     Hz out", 1, 4);
    #endif
    tone(pulseOutputPin, freqArr[freqIndex]);
    pinMode(pulseInputPin, INPUT_PULLUP);
  #endif
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
  #ifdef PULSE_OUTPUT_MODE
    if(freqTime <= time){
      int indexMax = sizeof(freqArr)/sizeof(int);
      freqIndex = (++freqIndex)%indexMax;
      tone(pulseOutputPin, freqArr[freqIndex]);
      #ifdef DEBUG_MODE
        myPrintLong(freqArr[freqIndex], 1, 4);
      #endif
      freqTime += FREQ_DURATION;
    }
  #endif
  
  // 0.5秒ごと周波数算出・出力
  if(freqOutputTime <= time){
    // 割り込み中断
		noInterrupts();
    long spansTotal = pulseSpans;
    long pulseNum = counter;
    pulseSpans = 0;
    // 割り込み再開
		interrupts();
    // 周波数算出
    long freqLong = (long)((pulseNum - beforePulseNum) * 1000000 / spansTotal);
    //buffer[FREQ] = (int)freqLong;
    //buffer[SPEED] = (int)(freqLong / 10);
    buffer[FREQ] = (buffer[FREQ]+1)%0XFF;
    buffer[SPEED] = (buffer[FREQ]+2)%0XFF;
    
    #ifdef DEBUG_MODE
      if(0 < freq){
        myPrintLong(long(freq/10), 1, 2);
        myPrintLong(freq, 1, 3);
      }
    #endif
    beforePulseNum = pulseNum;
    freqOutputTime += HALF_SECOND;
  }
}

volatile unsigned long beforeTime = 0;
// 割り込み処理（カウント）
void interruption(){
  // 立ち上がり時
  if(digitalRead(pulseInputPin) == HIGH){
    counter++;
    if(counter < 0){
      counter = 0;
    }
    // 波長をusで取得・加算
    unsigned long time = micros();
    pulseSpans += time - beforeTime;
    beforeTime = time;
  }
}

#ifndef DEBUG_MODE
  /**
  * I2C通信受信割り込み処理
  */
  void receiveEvent(int numByte){
    while(0 < Wire.available()){
      regIndex = Wire.read();
      if(2 < regIndex){
        regIndex = 0;
      }
    }
  }

  /**
   * I2C通信リクエスト割り込み処理
   */
  void requestEvent(){
    Wire.write(buffer[regIndex]);
  }
#endif

#ifdef DEBUG_MODE
  /**
  * long変数表示処理
  */  
  void myPrintLong(long value, int x, int y){
    char spacer = ' ';
    if(10000 <= value){
      spacer = '0';
      value = value%10000;
    }
    OzOled.setCursorXY(x,y);
    for(int d=1000; 0<d; d/=10){
      if(value/d == 0){
        OzOled.printChar(spacer);
      }
      else{
        OzOled.printNumber(value);
        break;
      }
    }
  }
#endif