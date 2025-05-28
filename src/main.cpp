#include <M5Unified.h> 
#include <ESP32Servo.h>

Servo servo1; // サーボオブジェクト作成
int servo1Pin = 5; //ピンG5を指定

// サーボモータ設定
int minUs = 500;
int maxUs = 2400;

//サーボモータ角度
int pos = 0; 

void setup() {
// M5Stack初期設定用の構造体を代入
 auto cfg = M5.config();       

 // M5デバイスの初期化
 M5.begin(cfg);                          

//サーボモータ初期化
  servo1.setPeriodHertz(50);  
  servo1.attach(servo1Pin, minUs, maxUs);
}

void loop() {
  M5.update();

  // ボタンを押した時の動作
  if (M5.BtnA.wasPressed()) {
    // サーボモータを0-90°まで1°ずつスイープする。
    for (pos = 0; pos <= 90; pos += 1) { 
      servo1.write(pos);
      delay(10);    
    }

    // 1秒待機
    delay(1000);

    // サーボモータを90°-0°まで1°ずつスイープする。
    for (pos = 90; pos >= 0; pos -= 1) {
      servo1.write(pos);
      delay(10);
    }
  }

  delay(1);
}