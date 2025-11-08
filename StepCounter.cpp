#include "StepCounter.h"
#include <math.h>

StepCounter::StepCounter() {}
StepCounter::StepCounter(MotionSensor *sensor) : imu(sensor) {}

void StepCounter::begin() {
    stepCount = 0;
    lastStepTime = 0;
}

void StepCounter::enableHRValidation(HRValidator *validator) {
    hrValidator = validator;
    hrValidationEnabled = true;
}

void StepCounter::disableHRValidation() {
    hrValidator = nullptr;
    hrValidationEnabled = false;
}

void StepCounter::update(float ax, float ay, float az, float gx, float gy, float gz) {
    float accelMag = sqrt(ax * ax + ay * ay + az * az);
    float gyroMag = sqrt(gx * gx + gy * gy + gz * gz);

    if (detectStep(accelMag, gyroMag)) {
        bool hrOK = true;
        if (hrValidationEnabled && hrValidator) {
            hrOK = hrValidator->isActiveState();
        }
        if (hrOK) stepCount++;
    }
}

bool StepCounter::detectStep(float accelMag, float gyroMag) {
    // Thresholds can be tuned experimentally
    const float accelThresh = 1.15;  // g's
    const float gyroPeak = 150.0;    // deg/s (to reject shakes)
    unsigned long now = millis();

    bool stepDetected = (accelMag > accelThresh && gyroMag < gyroPeak);
    if (stepDetected && (now - lastStepTime) > 300) {
        lastStepTime = now;
        return true;
    }
    return false;
}

int StepCounter::getStepCount() {
    return stepCount;
}
