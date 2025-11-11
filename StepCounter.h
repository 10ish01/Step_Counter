#pragma once
#include <Arduino.h>

// ------------------------------
// StepCounter Class Declaration
// ------------------------------
class StepCounter {
public:
    StepCounter();

    void begin();
    void update(float ax, float ay, float az, float gx, float gy, float gz);

    uint32_t getStepCount();
    float getCadence() const;

    enum State { IDLE, WALKING, RUNNING };
    State getCurrentState() const;

private:
    // --- Internal State ---
    State currentState;
    uint32_t stepCount;
    bool stepDetected;

    // --- Thresholds and Parameters ---
    float accelThresh;    // Acceleration threshold (g)
    float gyroThresh;     // Gyroscope magnitude limit (°/s)
    unsigned long stepGap; // Minimum gap between valid steps (ms)

    // --- Timing ---
    unsigned long lastStepTime;
    unsigned long lastActiveTime;

    // --- Cadence Tracking ---
    static const int CADENCE_WINDOW = 5;
    unsigned long stepIntervals[CADENCE_WINDOW];
    unsigned long stepTimes[CADENCE_WINDOW];
    int intervalIndex;
    bool intervalFilled;

    // --- Walking Confirmation ---
    int stepBufferIndex;
    int totalStepsBuffer;
    bool walkingConfirmed;

    // --- Internal Helpers ---
    bool detectStep(float ax, float ay, float az, float gx, float gy, float gz);
    bool checkCadence(unsigned long now);
    void setState(State newState);
};

