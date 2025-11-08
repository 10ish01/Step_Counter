#pragma once
#include <Arduino.h>
#include "HRValidator.h"

// Optional forward declaration to use MotionSensor if available
class MotionSensor;

class StepCounter {
public:
    StepCounter();
    StepCounter(MotionSensor *sensor);   // if using external MotionSensor

    void begin();
    void update(float ax, float ay, float az, float gx = 0, float gy = 0, float gz = 0);
    int getStepCount();

    // Optional HR validation
    void enableHRValidation(HRValidator *validator);
    void disableHRValidation();

private:
    MotionSensor *imu = nullptr;   // optional
    HRValidator *hrValidator = nullptr;
    bool hrValidationEnabled = false;

    int stepCount = 0;
    unsigned long lastStepTime = 0;

    bool detectStep(float mag, float gyroMag);
};
