#include <M5Unified.h>
#include <ESP32Servo.h>

Servo servo1, servo2, servo3, servo4;
int servoPins[] = {5, 6, 7, 8};
int minUs = 500, maxUs = 2400;

float accX, accY, accZ;
const char* prevAxis = "";
unsigned long stableStartTime = 0;
const unsigned long stableDuration = 4000;

void moveServosOneByOne(const int fromDeg[4], const int toDeg[4], uint16_t stepDelay = 30) {
  Servo* sv[4] = {&servo1, &servo2, &servo3, &servo4};
  for (int i = 0; i < 4; ++i) {
    int step = (toDeg[i] > fromDeg[i]) ? 1 : -1;
    for (int pos = fromDeg[i]; pos != toDeg[i] + step; pos += step) {
      sv[i]->write(pos);
      delay(stepDelay);
    }
    delay(100);
  }
}

const char* getMaxAccelAxis(float& maxAccel) {
  maxAccel = accX;
  const char* axis = "X";
  if (abs(accY) > abs(maxAccel)) { maxAccel = accY; axis = "Y"; }
  if (abs(accZ) > abs(maxAccel)) { maxAccel = accZ; axis = "Z"; }
  return axis;
}

void displayMaxAccel(const char* axis, float maxAccel) {
  M5.Display.clear();
  M5.Display.setCursor(0, 0);
  if (maxAccel < 0) M5.Display.printf("Max Axis: -%s\nAccel: %.2f\n", axis, maxAccel);
  else              M5.Display.printf("Max Axis: %s\nAccel: %.2f\n", axis, maxAccel);
}

bool isStable(const char* axis) {
  if (axis == prevAxis) {
    if (stableStartTime == 0) stableStartTime = millis();
    else if (millis() - stableStartTime >= stableDuration) return true;
  } else {
    stableStartTime = 0;
  }
  prevAxis = axis;
  return false;
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  servo1.setPeriodHertz(50); servo1.attach(servoPins[0], minUs, maxUs);
  servo2.setPeriodHertz(50); servo2.attach(servoPins[1], minUs, maxUs);
  servo3.setPeriodHertz(50); servo3.attach(servoPins[2], minUs, maxUs);
  servo4.setPeriodHertz(50); servo4.attach(servoPins[3], minUs, maxUs);

  if (!M5.Imu.begin()) { while (1) delay(1000); }
}

void loop() {
  M5.update();
  M5.Imu.getAccel(&accX, &accY, &accZ);

  float maxAccel;
  const char* axis = getMaxAccelAxis(maxAccel);
  displayMaxAccel(axis, maxAccel);

  if (isStable(axis) && !(axis == "Z" && maxAccel < 0)) {
    int startDeg[4] = {90, 80, 82, 85};
    int goalDeg[4]  = {0,  0,  0,  0};
    moveServosOneByOne(startDeg, goalDeg, 30);
    delay(2000);
    moveServosOneByOne(goalDeg, startDeg, 30);
  }
  delay(100);
}
