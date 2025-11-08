#include "MotionSensor.h"
#include "StepCounter.h"
#include "MAX30102Validator.cpp"

MotionSensor mpu;
StepCounter stepCounter(&mpu);
MAX30102Validator hr;

void setup() {
  Serial.begin(115200);
  mpu.begin();
  hr.begin();
  stepCounter.begin();
  stepCounter.enableHRValidation(&hr);
}

void loop() {
  mpu.update();
  hr.update();

  stepCounter.update(mpu.getAccelX(), mpu.getAccelY(), mpu.getAccelZ(),
                     mpu.getGyroX(), mpu.getGyroY(), mpu.getGyroZ());

  Serial.println(stepCounter.getStepCount());
  delay(50);
}
