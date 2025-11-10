#include "StepCounter.h"
#include <math.h>

StepCounter::StepCounter() {}
StepCounter::StepCounter(MotionSensor *sensor) : imu(sensor) {}

void StepCounter::begin() {
    stepCount = 0;
    lastStepTime = millis();
}

void StepCounter::update(float ax, float ay, float az, float gx, float gy, float gz) {
    float accelMag = sqrt(ax * ax + ay * ay + az * az);
    float gyroMag  = sqrt(gx * gx + gy * gy + gz * gz);

    if (detectStep(accelMag, gyroMag)) {
        bool hrOK = true;
        if (hrValidationEnabled && hrValidator)
            hrOK = hrValidator->isActiveState();
        if (hrOK) stepCount++;
    }
}

bool StepCounter::detectStep(float accelMag, float gyroMag) {
    unsigned long now = millis();

    // Hysteresis-based detection
    if (!stepActive && accelMag > upperThresh && gyroMag < gyroPeak) {
        stepActive = true;  // rising phase
    } 
    else if (stepActive && accelMag < lowerThresh) {
        stepActive = false; // falling phase — confirm step
        if (now - lastStepTime > minStepGap) {
            lastStepTime = now;
            return true;
        }
    }
    return false;
}

// --- Threshold Configuration ---
void StepCounter::setThresholds(float upper, float lower, float gyro) {
    if (upper > lower && upper > 1.0 && lower > 0.5 && gyro > 50) {
        upperThresh = upper;
        lowerThresh = lower;
        gyroPeak = gyro;
    }
}

void StepCounter::resetThresholds() {
    upperThresh = 1.20;
    lowerThresh = 0.95;
    gyroPeak = 150.0;
}

// --- HR Validation ---
void StepCounter::enableHRValidation(HRValidator *validator) {
    hrValidator = validator;
    hrValidationEnabled = true;
}

void StepCounter::disableHRValidation() {
    hrValidator = nullptr;
    hrValidationEnabled = false;
}

int StepCounter::getStepCount() {
    return stepCount;
}
