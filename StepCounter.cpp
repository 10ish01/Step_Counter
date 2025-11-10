#include "StepCounter.h"
#include <math.h>

StepCounter::StepCounter()
    : currentState(IDLE),
      stepCount(0),
      stepDetected(false),
      accelThresh(1.15),
      gyroThresh(150),
      stepGap(300),
      lastStepTime(0),
      lastActiveTime(0),
      intervalIndex(0),
      intervalFilled(false)
{
    for (int i = 0; i < CADENCE_WINDOW; i++) stepIntervals[i] = 0;
}

void StepCounter::begin() {
    stepCount = 0;
    currentState = IDLE;
    intervalIndex = 0;
    intervalFilled = false;
}

void StepCounter::update(float ax, float ay, float az, float gx, float gy, float gz) {
    bool isStep = detectStep(ax, ay, az, gx, gy, gz);
    unsigned long now = millis();

    switch (currentState) {
        case IDLE:
            if (isStep && (now - lastActiveTime > 500)) {  // hysteresis to avoid false start
                setState(WALKING);
                stepCount++;
                lastStepTime = now;
            }
            break;

        case WALKING:
        case RUNNING:
            if (isStep && (now - lastStepTime > stepGap)) {
                stepCount++;

                unsigned long interval = now - lastStepTime;
                stepIntervals[intervalIndex] = interval;
                intervalIndex = (intervalIndex + 1) % CADENCE_WINDOW;
                if (intervalIndex == 0) intervalFilled = true;

                lastStepTime = now;
            }
            if ((now - lastStepTime) > 2000) {  // 2s inactivity → IDLE
                setState(IDLE);
            }
            break;
    }

    if (isStep)
        lastActiveTime = now;
}

bool StepCounter::detectStep(float ax, float ay, float az, float gx, float gy, float gz) {
    float aMag = sqrt(ax * ax + ay * ay + az * az);
    float gMag = sqrt(gx * gx + gy * gy + gz * gz);
    return (aMag > accelThresh && gMag < gyroThresh);
}

void StepCounter::setState(State newState) {
    if (newState != currentState)
        currentState = newState;
}

uint32_t StepCounter::getStepCount() {
    return stepCount;
}

StepCounter::State StepCounter::getCurrentState() const {
    return currentState;
}

// Get Walking Pace ---
float StepCounter::getCadence() const {
    int count = intervalFilled ? CADENCE_WINDOW : intervalIndex;
    if (count == 0) return 0.0f;

    unsigned long sum = 0;
    for (int i = 0; i < count; i++) sum += stepIntervals[i];
    float avgIntervalMs = (float)sum / count;

    // steps per minute = 60000 / avgIntervalMs
    return (avgIntervalMs > 0) ? (60000.0f / avgIntervalMs) : 0.0f;
}
