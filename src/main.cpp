#include <M5Unified.h> 
#include <ESP32Servo.h>

Servo servo1; // サーボオブジェクト作成
int servo1Pin = 5; //ピンG5を指定

// サーボモータ設定
int minUs = 500;
int maxUs = 2400;

//サーボモータ角度
int pos = 0; 

float accX, accY, accZ; // 加速度の値を格納する変数

// 最大加速度の向きを判定する関数
const char* getMaxAccelAxis(float& maxAccel) {
  maxAccel = abs(accX);
  const char* axis = "X";
  if (abs(accY) > maxAccel) {
    maxAccel = abs(accY);
    axis = "Y";
  }
  if (abs(accZ) > maxAccel) {
    maxAccel = abs(accZ);
    axis = "Z";
  }
  return axis;
}

// 最大加速度の向きを画面に表示する関数
void displayMaxAccel(const char* axis, float maxAccel) {
  M5.Display.clear();
  M5.Display.setCursor(0, 0);
  M5.Display.printf("Max Axis: %s\nAccel: %.2f\n", axis, maxAccel);
}

// サーボモータを動作させる関数
void moveServo() {
  for (pos = 0; pos <= 90; pos += 1) { 
    servo1.write(pos);
    delay(30);    
  }

  delay(2000);

  for (pos = 90; pos >= 0; pos -= 1) {
    servo1.write(pos);
    delay(30);
  }
}

void setup() {
  auto cfg = M5.config();       
  M5.begin(cfg);                          
  servo1.setPeriodHertz(50);  
  servo1.attach(servo1Pin, minUs, maxUs);

  if (!M5.Imu.begin()) {
    M5.Display.clear();
    M5.Display.setCursor(0, 0);
    M5.Display.println("IMU Init Failed!");
    while (1) {
      delay(1000);
    }
  }
}

void loop() {
  M5.update();

  // IMUから加速度を取得
  M5.Imu.getAccel(&accX, &accY, &accZ);

  // 最大加速度の向きを判定
  float maxAccel;
  const char* axis = getMaxAccelAxis(maxAccel);

  // 最大加速度の向きを画面に表示
  displayMaxAccel(axis, maxAccel);

  // 最大加速度の向きがZ軸かつ正の値ではない場合にサーボを動かす
  if (!(axis == "Z" && accZ > 0)) {
    moveServo();
  }

  delay(5000); // 画面更新間隔
}