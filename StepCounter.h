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
    unsigned long bufferedSteps = 0;
    int unstableFrames = 0;

    // --- Cadence tracking ---
    static const int CADENCE_WINDOW = 10; // number of steps used for average cadence
    unsigned long stepIntervals[CADENCE_WINDOW];
    unsigned long stepTimes[CADENCE_WINDOW];
    int intervalIndex = 0;
    bool intervalFilled = false;
    int stepBufferIndex = 0;
    int lowCadenceFrames = 0;
    
    // --- Walking state ---
    bool walking = false;

    // --- Tuning ---
    float accelThresh_entry = 0.15f; // g
    float gyroThresh_entry = 10.0f; // deg/s
    float accelThresh_walk= 0.1f; // g
    float gyroThresh_walk = 15.0f; // deg/s
    unsigned long stepGap = 250; // ms minimum between steps
    int spikePenalty;                  // accumulates for detected spikes
    unsigned long spikeDecayTimer;     // last decay time for penalty
     
};
