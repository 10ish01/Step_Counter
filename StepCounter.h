#pragma once
#include <Arduino.h>

class StepCounter {
public:
    StepCounter();

    void begin();
    void update(float ax, float ay, float az, float gx, float gy, float gz);

    unsigned long getStepCount() const;
    float getCadence() const;
    bool isWalking() const;

private:
    // --- Step detection core ---
    bool detectStep(float ax, float ay, float az, float gx, float gy, float gz);

    // --- Step data ---
    unsigned long stepCount = 0;
    unsigned long lastStepTime = 0;
    unsigned long lastActiveTime = 0;

    // --- Cadence tracking ---
    static const int CADENCE_WINDOW = 5; // number of steps used for average cadence
    unsigned long stepIntervals[CADENCE_WINDOW];
    unsigned long stepTimes[CADENCE_WINDOW];
    int intervalIndex = 0;
    bool intervalFilled = false;
    int stepBufferIndex = 0;
    int totalStepsBuffer = 0;

    // --- Walking state ---
    bool walking = false;

    // --- Tuning ---
    float accelThresh = 0.1f; // g
    float gyroThresh = 10.0f; // deg/s
    unsigned long stepGap = 350; // ms minimum between steps
};
