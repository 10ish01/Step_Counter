#pragma once
#include <Arduino.h>

class StepCounter {
public:
    StepCounter();
    void begin();
    void update(float ax, float ay, float az, float gx, float gy, float gz);
    uint32_t getStepCount();
    float getCadence() const;
    void setState(State newState);
    enum State { IDLE, WALKING, RUNNING };
    State getCurrentState() const;

private:
    // --- Step detection ---
    bool detectStep(float ax, float ay, float az, float gx, float gy, float gz);

    // --- Cadence logic ---
    bool checkCadence(unsigned long now);

    // --- Variables ---
    State currentState;
    uint32_t stepCount;
    bool stepDetected;

    float accelThresh;
    float gyroThresh;
    unsigned long stepGap;
    unsigned long lastStepTime;
    unsigned long lastActiveTime;

    // --- Cadence estimation ---
    static const int CADENCE_WINDOW = 6;
    unsigned long stepIntervals[CADENCE_WINDOW];
    unsigned long stepTimes[CADENCE_WINDOW];
    int intervalIndex;
    bool intervalFilled;
    int stepBufferIndex;
    int totalStepsBuffer;
    bool walkingConfirmed;
};
