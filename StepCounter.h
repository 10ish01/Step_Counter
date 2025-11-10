#pragma once
#include <Arduino.h>

class StepCounter {
public:
    enum State {
        IDLE,
        WALKING,
        RUNNING
    };

    StepCounter();

    void begin();
    void update(float ax, float ay, float az, float gx, float gy, float gz);
    uint32_t getStepCount();

    // --- NEW ---
    State getCurrentState() const;
    float getCadence() const;   // <-- returns average steps per minute

private:
    State currentState;

    uint32_t stepCount;
    unsigned long lastStepTime;
    unsigned long lastActiveTime;

    float accelThresh;
    float gyroThresh;
    unsigned long stepGap;

    bool stepDetected;


    static const int CADENCE_WINDOW = 5; // last 5 step intervals
    unsigned long stepIntervals[CADENCE_WINDOW];
    int intervalIndex;
    bool intervalFilled;

    bool detectStep(float ax, float ay, float az, float gx, float gy, float gz);
    void setState(State newState);
};
