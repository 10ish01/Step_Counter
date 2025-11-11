#include "StepCounter.h"
#include <math.h>

StepCounter::StepCounter() {}

void StepCounter::begin() {
    stepCount = 0;
    lastStepTime = 0;
    intervalIndex = 0;
    intervalFilled = false;
    walking = false;
}

// --- Step detection based on thresholds ---
bool StepCounter::detectStep(float ax, float ay, float az, float gx, float gy, float gz) {
    float aMag = sqrt(ax * ax + ay * ay + az * az);
    float gMag = sqrt(gx * gx + gy * gy + gz * gz);
    return (aMag > accelThresh && gMag < gyroThresh);
}

void StepCounter::update(float ax, float ay, float az, float gx, float gy, float gz) {
    bool isStep = detectStep(ax, ay, az, gx, gy, gz);
    unsigned long now = millis();

    // --- If a new step is detected ---
    if (isStep && (now - lastStepTime > stepGap)) {
        unsigned long interval = (lastStepTime > 0) ? now - lastStepTime : 0;
        lastStepTime = now;
        lastMovementTime = now;
        if (walking) stepCount++;

        // --- Record interval for cadence calculation ---
        if (interval > 0) {
            stepIntervals[intervalIndex] = interval;
            intervalIndex = (intervalIndex + 1) % CADENCE_WINDOW;
            if (intervalIndex == 0) intervalFilled = true;
        }

        // --- Check cadence once enough steps accumulated ---
        if (intervalFilled) {
            float cadence = getCadence();
            if (!walking && cadence >= 60.0f && cadence <= 160.0f) {
                walking = true;
                // retroactively add the first window of steps if needed
                stepCount += CADENCE_WINDOW;
            }
        }
    }

    // --- Stop walking after inactivity ---
    if (walking && (now - lastMovementTime > 2000)) {
        walking = false;
    }
}

// --- Getters ---
unsigned long StepCounter::getStepCount() const {
    return stepCount;
}

float StepCounter::getCadence() const {
    int count = intervalFilled ? CADENCE_WINDOW : intervalIndex;
    if (count == 0) return 0.0f;

    unsigned long sum = 0;
    for (int i = 0; i < count; i++) sum += stepIntervals[i];
    float avgIntervalMs = (float)sum / count;

    if (!walking) return 0.0f;  // cadence = 0 if idle
    return (avgIntervalMs > 0) ? (60000.0f / avgIntervalMs) : 0.0f;
}

bool StepCounter::isWalking() const {
    return walking;
}
