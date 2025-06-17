#include <M5Unified.h>
#include <ESP32Servo.h>

Servo sv1, sv2, sv3, sv4;
constexpr int pins[4] = {5, 6, 7, 8};
constexpr int minUs = 500, maxUs = 2400;

float ax, ay, az;
const char* prevA = "";
unsigned long stTime = 0;
constexpr unsigned long stDur = 4000;

void moveOneByOne(const int fromDeg[4], const int toDeg[4], const uint16_t dly[4]) {
  Servo* s[4] = {&sv1, &sv2, &sv3, &sv4};
  for (int i = 0; i < 4; ++i) {
    int step = (toDeg[i] > fromDeg[i]) ? 1 : -1;
    for (int p = fromDeg[i]; p != toDeg[i] + step; p += step) {
      s[i]->write(p);
      delay(dly[i]);
    }
    delay(100);
  }
}

const char* maxAxis(float& m) {
  m = ax; const char* a = "X";
  if (abs(ay) > abs(m)) { m = ay; a = "Y"; }
  if (abs(az) > abs(m)) { m = az; a = "Z"; }
  return a;
}

bool stable(const char* a) {
  if (a == prevA) {
    if (!stTime) stTime = millis();
    else if (millis() - stTime >= stDur) return true;
  } else stTime = 0;
  prevA = a;
  return false;
}

void setup() {
  auto cfg = M5.config(); M5.begin(cfg);
  sv1.setPeriodHertz(50); sv1.attach(pins[0], minUs, maxUs);
  sv2.setPeriodHertz(50); sv2.attach(pins[1], minUs, maxUs);
  sv3.setPeriodHertz(50); sv3.attach(pins[2], minUs, maxUs);
  sv4.setPeriodHertz(50); sv4.attach(pins[3], minUs, maxUs);
  if (!M5.Imu.begin()) while (1) delay(1000);
}

void loop() {
  M5.update();
  M5.Imu.getAccel(&ax, &ay, &az);

  float m; const char* a = maxAxis(m);

  if (stable(a) && !(a == "Z" && m < 0)) {
    int start[4] = {80, 80, 82, 80};
    int goal[4]  = { 0,  0,  0,  0};
    uint16_t spd[4] = {10, 10, 10, 10};  // pin5のみ速い
    moveOneByOne(start, goal, spd);
    delay(2000);
    moveOneByOne(goal, start, spd);
  }
  delay(100);
}
