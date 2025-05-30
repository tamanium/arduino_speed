/*
      ┏━━┓_┏━━┓
Reset ┃o      ┃ VCC
Pin3  ┃       ┃ Pin2 SCK
Pin4  ┃       ┃ Pin1
GND   ┃       ┃ Pin0 SDA
      ┗━━━━━━━┛
*/

//#define DEBUG_MODE
#define PULSE_OUTPUT_MODE
#ifdef DEBUG_MODE
  #include <TinyWireM.h>
  #include <TinyOzOLED.h>
#else
  #include <Wire.h>
#endif
#include <PinChangeInterrupt.h>

// 速度パルス検知ピン
#define pulseInputPin 4
// パルス出力ピン
#define pulseOutputPin 1
// 電圧測定ピン
#define adcInputPin 3
// I2Cアドレス
#define address 0x55

// 定数
const int HALF_SECOND = 527;
const int FREQ_DURATION = HALF_SECOND * 10;
const int FREQ_MAX = 10000;
// レジスタアドレス
const byte FREQ_IN  = 0x00;
const byte FREQ_OUT = 0x01;
const byte ADC_IN   = 0x02;
// 変数
long counter = 0;
int dataBuffer[3] = {-1, -1, 0};

byte regIndex = 0;

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
  1100
};
int freqArrSize = sizeof(freqArr)/sizeof(int);

int freqIndex = -1;

void setup(){
  #ifdef DEBUG_MODE
    OzOled.init();
    OzOled.printString("monitor", 1, 0);
    OzOled.printString("     km/h", 1, 2);
    OzOled.printString("     Hz in", 1, 3);
    OzOled.printString("     pt adc", 1, 5);
  #else
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


unsigned long beforeCounter = 0;  // 前回総パルス数
unsigned long beforePulseNum = 0; // 前回単位時間あたりパルス数
unsigned long freqOutputTime = 0; // 周波数出力時間
unsigned long adcInputTime = 0;   // 電圧測定時間
unsigned long freqTime = 0;       // 周波数変更時間
unsigned long pulseSpans = 0;     // 単位時間あたり波長合計

void loop(){
  // システム時刻取得(ms)
  unsigned long time = millis();

  // 周波数変更
  #ifdef PULSE_OUTPUT_MODE
    if(freqTime <= time){
      // 周波数配列インデックスをインクリメント
      freqIndex = (++freqIndex)%freqArrSize;
      // 出力周波数変更
      tone(pulseOutputPin, freqArr[freqIndex]);
      // 出力バッファに上記周波数値を代入
      dataBuffer[FREQ_OUT] = freqArr[freqIndex]%FREQ_MAX;
      #ifdef DEBUG_MODE
        myPrintLong(dataBuffer[FREQ_OUT], 1, 4);
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
    int freqInt = (int)((pulseNum - beforePulseNum) * 1000000 / spansTotal);
    dataBuffer[FREQ_IN] = (freqInt)%FREQ_MAX;
    
    #ifdef DEBUG_MODE
      if(0 < freqInt){
        myPrintLong(long(freqInt/10), 1, 2);
        myPrintLong(dataBuffer[FREQ_IN], 1, 3);
      }
    #endif
    beforePulseNum = pulseNum;
    freqOutputTime += HALF_SECOND;
  }

  // 1秒ごとに電圧測定
  if(adcInputTime <= time){
    dataBuffer[ADC_IN] = analogRead(adcInputPin);
    #ifdef DEBUG_MODE
      myPrintLong(dataBuffer[ADC_IN], 1, 5);
    #endif
    adcInputTime += HALF_SECOND * 2;
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

#ifdef DEBUG_MODE
	/**
	 * long変数表示処理
   */  
  void myPrintLong(long value, int x, int y){
    char spacer = ' ';
    if(FREQ_MAX <= value){
      spacer = '0';
      value = value%FREQ_MAX;
    }
    OzOled.setCursorXY(x,y);
    for(int d=FREQ_MAX; 1<d; d/=10){
      if(value/d == 0){
        OzOled.printChar(spacer);
      }
      else{
        OzOled.printNumber(value);
        return;
      }
    }
  }
#else
  /**
   * I2C通信受信割り込み処理
   */
  void receiveEvent(int numByte){
    while(0 < Wire.available()){
      regIndex = Wire.read();
    }
  }

  /**
   * I2C通信リクエスト割り込み処理
   */
  void requestEvent(){ 
    int sendData = -1;
    if(regIndex==FREQ_IN
      || regIndex==FREQ_OUT
      || regIndex==ADC_IN){
      sendData = dataBuffer[regIndex];
    }
    
    byte sendDataArr[2] = {byte(sendData>>8), byte(sendData&0xFF)};
    Wire.write(sendDataArr, 2);
  }
#endif