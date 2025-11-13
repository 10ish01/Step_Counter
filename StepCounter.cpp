#include "StepCounter.h"
#include <math.h>

StepCounter::StepCounter() {}

void StepCounter::begin() {
    stepCount = 0;
    lastStepTime = 0;
    intervalIndex = 0;
    intervalFilled = false;
    walking = false;
    bufferedSteps = 0;
    unstableFrames = 0;
    spikePenalty = 0;         
    spikeDecayTimer = 0;      
    memset(stepTimes, 0, sizeof(stepTimes));
}

bool StepCounter::detectStep(float ax, float ay, float az, float gx, float gy, float gz) { 
    static float lastAMag = 1.0f; // gravity baseline 
    static float lastGMag = 0.0f;

    float aMag = sqrt(ax * ax + ay * ay + az * az); 
    float gMag = sqrt(gx * gx + gy * gy + gz * gz); 

    float deltaA = fabs(aMag - lastAMag); 
    float deltaG = fabs(gMag - lastGMag); 

    lastAMag = aMag; 
    lastGMag = gMag;  

    float aThresh = walking ? accelThresh_walk : accelThresh_entry; 
    float gThresh = walking ? gyroThresh_walk : gyroThresh_entry; 

    // --- Spike detection ---
    if (deltaA > aThresh && deltaG > gThresh) {
        spikePenalty++;
        spikeDecayTimer = millis();
        return false; // reject as spike
    }

    return (deltaA > aThresh && deltaG < gThresh);
}


// --- Step detection and state logic ---
void StepCounter::update(float ax, float ay, float az, float gx, float gy, float gz) {
    bool isStepRaw = detectStep(ax, ay, az, gx, gy, gz);
    unsigned long now = millis();

    // --- Gradually decay spike penalty over time ---
    if (now - spikeDecayTimer > 3000 && spikePenalty > 0) {
        spikePenalty--;
        spikeDecayTimer = now;
    }

    // --- Adaptive penalties ---
    float stepGapAdj = stepGap;
    float accelAdj = walking ? accelThresh_walk : accelThresh_entry;

    if (spikePenalty > 3) {
        stepGapAdj *= 1.3f;
        accelAdj *= 1.1f;
    }

    unsigned long maxStepGap = walking ? 2500 : 2000;
    if (isStepRaw && (now - lastStepTime) > stepGapAdj && (now - lastStepTime) < maxStepGap) {
        lastStepTime = now;

        // Update rolling window
        for (int i = 1; i < CADENCE_WINDOW; i++)
            stepTimes[i - 1] = stepTimes[i];
        stepTimes[CADENCE_WINDOW - 1] = now;

        float cadence = getCadence();

        // --- Variance consistency check ---
        float variance = 0;
        int valid = 0;
        if (CADENCE_WINDOW >= 3) {
            float avg = 0;
            float intervals[CADENCE_WINDOW - 1];
            for (int i = 1; i < CADENCE_WINDOW; i++) {
                if (stepTimes[i] > stepTimes[i-1]) {
                    intervals[valid] = stepTimes[i] - stepTimes[i-1];
                    avg += intervals[valid];
                    valid++;
                }
            }
            if (valid >= 2) {
                avg /= valid;
                for (int i = 0; i < valid; i++)
                    variance += (intervals[i] - avg) * (intervals[i] - avg);
                variance /= valid;
            }
        }

        if (variance > 80000) return;

        // --- Walking entry logic ---
        if (!walking) {
            if (cadence >= 35 && cadence <= 120)
                bufferedSteps++;
            else
                bufferedSteps = 0;

            if (bufferedSteps >= 5) {
                walking = true;
                stepCount += bufferedSteps;
                bufferedSteps = 0;
            }
        } else {
            stepCount++;
        }
    }

    // --- Walking exit logic ---

if (walking) {
    if ((now - lastStepTime) > 2500) {       // no steps for 2.5s
        walking = false;
        lowCadenceFrames = 0;
    } else {
        float cadence = getCadence();

        if (cadence < 35 || cadence > 150)
            lowCadenceFrames++;
        else
            lowCadenceFrames = 0;

        if (lowCadenceFrames >= 3) {
            walking = false;
            lowCadenceFrames = 0;
        }
    }
}
 
}

// --- Getters ---
unsigned long StepCounter::getStepCount() const {
    return stepCount;
}

float StepCounter::getCadence() const {
    const unsigned long WINDOW_MS = walking ? 4000 : 6000;
    const unsigned long now = millis();

    if (now - lastStepTime > 2500) return 0.0f;

    unsigned long totalDelta = 0;
    int validPairs = 0;
    unsigned long prevTime = 0;
    unsigned long intervals[CADENCE_WINDOW];
    int intervalCount = 0;

    for (int i = 0; i < CADENCE_WINDOW; i++) {
        if (stepTimes[i] > 0 && (now - stepTimes[i]) <= WINDOW_MS) {
            if (prevTime > 0) {
                unsigned long diff = stepTimes[i] - prevTime;
                if (diff > 200 && diff < 3000) {
                    intervals[intervalCount++] = diff;
                    totalDelta += diff;
                }
            }
            prevTime = stepTimes[i];
        }
    }

    if (intervalCount < 2) return 0.0f;

    float avgInterval = (float)totalDelta / intervalCount;
    unsigned long filteredSum = 0;
    int filteredCount = 0;
    for (int i = 0; i < intervalCount; i++) {
        if (fabs((float)intervals[i] - avgInterval) < 800) {
            filteredSum += intervals[i];
            filteredCount++;
        }
    }

    if (filteredCount == 0) return 0.0f;

    float filteredAvg = (float)filteredSum / filteredCount;
    float instantCadence = 60000.0f / filteredAvg;

    static float smoothCadence = 0;
    const float alpha = 0.6f;
    smoothCadence = alpha * instantCadence + (1 - alpha) * smoothCadence;

    return smoothCadence;
}

bool StepCounter::isWalking() const {
    return walking;
}
