#include <M5Unified.h>
#include <ESP32Servo.h>

Servo servo1, servo2, servo3, servo4;
int servoPins[] = {5, 6, 7, 8};
int minUs = 500, maxUs = 2400;

float accX, accY, accZ;
const char* prevAxis = "";
unsigned long stableStartTime = 0;
const unsigned long stableDuration = 4000;

/* ★1: 個別角度対応・同期移動関数 ------------- */
void moveServosTogether(const int fromDeg[4], const int toDeg[4],
                        uint16_t stepDelay = 30) {
  int maxDelta = 0;
  for (int i = 0; i < 4; ++i)
    maxDelta = max(maxDelta, abs(toDeg[i] - fromDeg[i]));

  for (int s = 0; s <= maxDelta; ++s) {
    servo1.write(fromDeg[0] + (toDeg[0] - fromDeg[0]) * s / maxDelta);
    servo2.write(fromDeg[1] + (toDeg[1] - fromDeg[1]) * s / maxDelta);
    servo3.write(fromDeg[2] + (toDeg[2] - fromDeg[2]) * s / maxDelta);
    servo4.write(fromDeg[3] + (toDeg[3] - fromDeg[3]) * s / maxDelta);
    delay(stepDelay);
  }
}

const char* getMaxAccelAxis(float& maxAccel) {
  maxAccel = accX; const char* axis = "X";
  if (abs(accY) > abs(maxAccel)) { maxAccel = accY; axis = "Y"; }
  if (abs(accZ) > abs(maxAccel)) { maxAccel = accZ; axis = "Z"; }
  return axis;
}

void displayMaxAccel(const char* axis, float maxAccel) {
  M5.Display.clear(); M5.Display.setCursor(0, 0);
  if (maxAccel < 0)
    M5.Display.printf("Max Axis: -%s\nAccel: %.2f\n", axis, maxAccel);
  else
    M5.Display.printf("Max Axis: %s\nAccel: %.2f\n", axis, maxAccel);
}

bool isStable(const char* axis) {
  if (axis == prevAxis) {
    if (stableStartTime == 0) stableStartTime = millis();
    else if (millis() - stableStartTime >= stableDuration) return true;
  } else stableStartTime = 0;
  prevAxis = axis;
  return false;
}

void setup() {
  auto cfg = M5.config();  M5.begin(cfg);
  servo1.setPeriodHertz(50); servo1.attach(servoPins[0], minUs, maxUs);
  servo2.setPeriodHertz(50); servo2.attach(servoPins[1], minUs, maxUs);
  servo3.setPeriodHertz(50); servo3.attach(servoPins[2], minUs, maxUs);
  servo4.setPeriodHertz(50); servo4.attach(servoPins[3], minUs, maxUs);

  if (!M5.Imu.begin()) {
    M5.Display.clear(); M5.Display.setCursor(0, 0);
    M5.Display.println("IMU Init Failed!"); while (1) delay(1000);
  }
}

void loop() {
  M5.update();
  M5.Imu.getAccel(&accX, &accY, &accZ);

  float maxAccel;
  const char* axis = getMaxAccelAxis(maxAccel);
  displayMaxAccel(axis, maxAccel);

  if (isStable(axis)) {
    if (!(axis == "Z" && maxAccel < 0)) {

      /* ★2: 好きな目標角度をここで指定 ------------- */
      int startDeg[4] = {80, 80, 82, 73};     // 行きの基準
      int goalDeg [4] = { 0, 0, 0, 0};     // 個別ターゲット

      /* ★3: 行って → 待って → 戻る ------------- */
      moveServosTogether(startDeg, goalDeg, 30);  // 行き
      delay(2000);                                // 2 秒待機
      moveServosTogether(goalDeg, startDeg, 30);  // 戻り
    }
  }
  delay(100);  // 更新間隔
}
