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
float prevMaxAccel = 0; // 前回の最大加速度値
const char* prevAxis = ""; // 前回の最大加速度の向き
unsigned long stableStartTime = 0; // 安定状態の開始時間
const unsigned long stableDuration = 4000; // 安定状態を維持する時間 (ミリ秒)

// サーボモータを動作させる関数
void moveServo() {
  // サーボモータを90°から0°まで1°ずつスイープする
  for (pos = 90; pos >= 0; pos -= 1) { 
    servo1.write(pos);
    delay(30);    
  }

  delay(2000); // 2秒待機

  // サーボモータを0°から90°まで1°ずつスイープする
  for (pos = 0; pos <= 90; pos += 1) {
    servo1.write(pos);
    delay(30);
  }
}

// 最大加速度の向きを判定する関数
const char* getMaxAccelAxis(float& maxAccel) {
  maxAccel = accX;
  const char* axis = "X";

  if (abs(accY) > abs(maxAccel)) {
    maxAccel = accY;
    axis = "Y";
  }
  if (abs(accZ) > abs(maxAccel)) {
    maxAccel = accZ;
    axis = "Z";
  }

  return axis;
}

// 最大加速度の向きを画面に表示する関数
void displayMaxAccel(const char* axis, float maxAccel) {
  M5.Display.clear();
  M5.Display.setCursor(0, 0);

  // 正負を考慮して軸名を表示
  if (maxAccel < 0) {
    M5.Display.printf("Max Axis: -%s\nAccel: %.2f\n", axis, maxAccel);
  } else {
    M5.Display.printf("Max Axis: %s\nAccel: %.2f\n", axis, maxAccel);
  }
}

// 安定状態を判定する関数
bool isStable(const char* axis) {
  // 最大加速度の向きが前回と同じか確認
  if (axis == prevAxis) {
    if (stableStartTime == 0) {
      stableStartTime = millis(); // 安定状態の開始時間を記録
    } else if (millis() - stableStartTime >= stableDuration) {
      return true; // 安定状態が維持されている
    }
  } else {
    stableStartTime = 0; // 安定状態が崩れた場合、タイマーをリセット
  }

  // 前回の最大加速度の向きを更新
  prevAxis = axis;

  return false;
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

  // 安定状態を判定してサーボを動作
  if (isStable(axis)) {
    if (!(axis == "Z" && maxAccel < 0)) {
      moveServo();
    }
  }

  delay(100); // 更新間隔
}