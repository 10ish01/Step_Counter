#pragma once
#include <Arduino.h>
#include "HRValidator.h"

class MotionSensor;

class StepCounter {
public:
    StepCounter();
    StepCounter(MotionSensor *sensor);

    void begin();  
    void update(float ax, float ay, float az, float gx = 0, float gy = 0, float gz = 0);
    int getStepCount();

    // HR validation
    void enableHRValidation(HRValidator *validator);
    void disableHRValidation();

    // Threshold configuration (runtime only)
    void setThresholds(float upper, float lower, float gyro);
    void resetThresholds();

    // Accessors
    float getUpperThresh() const { return upperThresh; }
    float getLowerThresh() const { return lowerThresh; }
    float getGyroPeak()   const { return gyroPeak; }

private:
    MotionSensor *imu = nullptr;
    HRValidator *hrValidator = nullptr;
    bool hrValidationEnabled = false;

    int stepCount = 0;
    unsigned long lastStepTime = 0;

    // Thresholds (runtime)
    float upperThresh = 1.20;
    float lowerThresh = 0.95;
    float gyroPeak = 150.0;

    const unsigned long minStepGap = 300;  // ms
    bool stepActive = false;

    bool detectStep(float accelMag, float gyroMag);
};
