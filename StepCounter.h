#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include "HRValidator.h"

class MotionSensor;

class StepCounter {
public:
    StepCounter();
    StepCounter(MotionSensor *sensor);

    void begin(int resetPin = -1);   // optional reset pin
    void update(float ax, float ay, float az, float gx = 0, float gy = 0, float gz = 0);
    int getStepCount();

    // HR validation
    void enableHRValidation(HRValidator *validator);
    void disableHRValidation();

    // Threshold configuration
    void setAccelThreshold(float threshold);
    void setGyroPeak(float peak);
    float getAccelThreshold() const;
    float getGyroPeak() const;
    void resetThresholdsToDefault(bool save = true);

    // EEPROM persistence
    void saveThresholdsToEEPROM();
    void loadThresholdsFromEEPROM();

private:
    MotionSensor *imu = nullptr;
    HRValidator *hrValidator = nullptr;
    bool hrValidationEnabled = false;

    int stepCount = 0;
    unsigned long lastStepTime = 0;

    float accelThresh = 1.15;
    float gyroPeak = 150.0;

    bool detectStep(float mag, float gyroMag);

    // EEPROM layout
    static constexpr int EEPROM_START_ADDR = 0;

    // Reset button support
    int resetButtonPin = -1;
};
