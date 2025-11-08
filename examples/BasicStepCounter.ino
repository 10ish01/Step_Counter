#include "StepCounter.h"

StepCounter stepCounter;

void setup() {
  Serial.begin(115200);
  stepCounter.begin();
}

void loop() {
  float ax = 0.01, ay = 0.02, az = 1.05; // example accel
  float gx = 5, gy = 3, gz = 2;          // example gyro

  stepCounter.update(ax, ay, az, gx, gy, gz);
  Serial.println(stepCounter.getStepCount());

  delay(50);
}
