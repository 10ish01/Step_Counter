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
    static float lastAMag = 1.0f;  // gravity baseline
    static float lastGMag = 0.0f;
    float aMag = sqrt(ax * ax + ay * ay + az * az);
    float gMag = sqrt(gx * gx + gy * gy + gz * gz);

    float deltaA = fabs(aMag - lastAMag);
    float deltaG = fabs(gMag - lastGMag);
    lastAMag = aMag;
    lastGMag = gMag;
    return (deltaA > accelThresh && deltaG < gyroThresh);
}

void StepCounter::update(float ax, float ay, float az, float gx, float gy, float gz) {
    bool isStepRaw = detectStep(ax, ay, az, gx, gy, gz);
    unsigned long now = millis();

    // --- debounce: accept a new "logged" step only if min gap passed ---
    bool isStep = false;
    if (isStepRaw) {
        if (lastStepTime == 0 || (now - lastStepTime) > stepGap) {
            isStep = true;
        } else {
            // Too close to previous accepted step -> treat as bounce and ignore
            isStep = false;
        }
    }

    // --- If a valid (debounced) step happened, record timestamp and interval ---
    static unsigned long bufferedSteps = 0; // counts steps before walking is confirmed

    if (isStep) {
        // record timestamp into circular buffer
        stepTimes[stepBufferIndex] = now;
        stepBufferIndex = (stepBufferIndex + 1) % CADENCE_WINDOW;
        if (totalStepsBuffer < CADENCE_WINDOW) totalStepsBuffer++;

        // update pairwise interval for display/use (optional)
        if (totalStepsBuffer > 1) {
            // index of previous timestamp (chronologically)
            int prevIdx = (stepBufferIndex - 2 + CADENCE_WINDOW) % CADENCE_WINDOW;
            unsigned long interval = now - stepTimes[prevIdx];

            // store interval in stepIntervals at same position as prevIdx
            stepIntervals[prevIdx] = interval;

            // mark intervalFilled when we've wrapped at least once
            if (stepBufferIndex == 0) intervalFilled = true;
        }

        // update the last accepted step time AFTER we've used it for debounce
        lastStepTime = now;
        lastActiveTime = now;

        // counting logic: buffer until walking confirmed
        if (!walking) {
            bufferedSteps++;
        } else {
            // already walking: increment step count normally
            stepCount++;
        }
    }

    // --- Walking detection: compute avg delta using chronological order of buffer ---
    if (!walking && totalStepsBuffer == CADENCE_WINDOW) {
        // compute average delta between consecutive timestamps in chronological order
        // oldest index: stepBufferIndex points to the next slot to be written,
        // so oldest = stepBufferIndex (after wrap) and newest = stepBufferIndex-1
        unsigned long totalDelta = 0;
        for (int k = 0; k < CADENCE_WINDOW - 1; ++k) {
            int idxA = (stepBufferIndex + k) % CADENCE_WINDOW;       // older
            int idxB = (stepBufferIndex + k + 1) % CADENCE_WINDOW;   // newer
            unsigned long delta = stepTimes[idxB] - stepTimes[idxA];
            totalDelta += delta;
        }
        float avgDelta = (float)totalDelta / (CADENCE_WINDOW - 1);

        if (avgDelta >= 380.0f && avgDelta <= 2000.0f) {
            // confirm walking and add buffered steps once
            walking = true;
            stepCount += 1;
            bufferedSteps = 0;
        } else {
            // not walking — keep buffer but don't count steps
         
        }
    }

    // --- Stop walking if inactive, and clear buffers to avoid stale cadence ---
    if (walking && (now - lastStepTime > 1500)) {
        walking = false;
        bufferedSteps = 0;
        totalStepsBuffer = 0;
        intervalIndex = 0;
        intervalFilled = false;
        // clear arrays to avoid stale values (optional but safe)
        for (int i = 0; i < CADENCE_WINDOW; ++i) {
            stepIntervals[i] = 0;
            stepTimes[i] = 0;
        }
        stepBufferIndex = 0;
    }
}


// --- Getters ---
unsigned long StepCounter::getStepCount() const {
    return stepCount;
}
float StepCounter::getCadence() const {
    unsigned long now = millis();
    int validCount = 0;
    unsigned long sumIntervals = 0;

    // Go through step times and keep only recent ones (within 5s)
    for (int i = 1; i < totalStepsBuffer; i++) {
        unsigned long t1 = stepTimes[(i - 1) % CADENCE_WINDOW];
        unsigned long t2 = stepTimes[i % CADENCE_WINDOW];
        if (now - t2 < 5000) {  // 5-second rolling window
            sumIntervals += (t2 - t1);
            validCount++;
        }
    }

    if (validCount == 0) return 0.0f;

    float avgIntervalMs = (float)sumIntervals / validCount;
    return (avgIntervalMs > 0) ? (60000.0f / avgIntervalMs) : 0.0f;
}

bool StepCounter::isWalking() const {
    return walking;
}
