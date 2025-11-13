#include "StepCounter.h"
#include "HRValidator.h"
#include <math.h>

// --- Constructor ---
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
    lowCadenceFrames = 0;
    currentCadence = 0;
    currentVariance = 0;
    memset(stepTimes, 0, sizeof(stepTimes));
}

void StepCounter::enableHRValidation(HRValidator* validator) {
    hrValidator = validator;
    hrValidationEnabled = true;
}

void StepCounter::disableHRValidation() {
    hrValidator = nullptr;
    hrValidationEnabled = false;
}

// --- Step detection core ---
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

// --- Unified cadence + variance computation ---
void StepCounter::computeCadenceAndVariance() {
    const unsigned long now = millis();
    const unsigned long WINDOW_MS = walking ? 4000 : 6000;

    // --- Reset if idle too long ---
    if (now - lastStepTime > 2500) {
        currentCadence = 0.0f;
        currentVariance = 0.0f;
        return;
    }

    unsigned long totalDelta = 0;
    unsigned long intervals[CADENCE_WINDOW];
    int intervalCount = 0;
    unsigned long prevTime = 0;

    // Collect valid intervals
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

    if (intervalCount < 2) {
        currentCadence = 0.0f;
        currentVariance = 0.0f;
        return;
    }

    // --- Average interval ---
    float avgInterval = (float)totalDelta / intervalCount;

    // --- Variance computation ---
    float var = 0.0f;
    for (int i = 0; i < intervalCount; i++) {
        float diff = (float)intervals[i] - avgInterval;
        var += diff * diff;
    }
    var /= intervalCount;
    currentVariance = var;

    // --- Filter outliers for cadence ---
    unsigned long filteredSum = 0;
    int filteredCount = 0;
    for (int i = 0; i < intervalCount; i++) {
        if (fabs((float)intervals[i] - avgInterval) < 800) {
            filteredSum += intervals[i];
            filteredCount++;
        }
    }

    if (filteredCount == 0) {
        currentCadence = 0.0f;
        return;
    }

    float filteredAvg = (float)filteredSum / filteredCount;
    float instantCadence = 60000.0f / filteredAvg;

    // --- Safety cap for cadence ---
    if (instantCadence < 20 || instantCadence > 200) {
        currentCadence = 0.0f;
        return;
    }

    // --- Exponential smoothing ---
    static float smoothCadence = 0;
    const float alpha = 0.6f;
    smoothCadence = alpha * instantCadence + (1 - alpha) * smoothCadence;

    currentCadence = smoothCadence;
}

// --- Main update loop ---
void StepCounter::update(float ax, float ay, float az, float gx, float gy, float gz) {
    bool isStepRaw = detectStep(ax, ay, az, gx, gy, gz);
    unsigned long now = millis();

    // --- Decay spike penalty ---
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

    // --- Partial adaptive step gap (based on cadence when walking) ---
    if (walking && currentCadence > 0) {
        stepGapAdj = constrain(0.4f * (60000.0f / currentCadence), 250.0f, 1200.0f);
    }

    unsigned long maxStepGap = walking ? 2500 : 2000;

    // --- Candidate step ---
    if (isStepRaw && (now - lastStepTime) > stepGapAdj && (now - lastStepTime) < maxStepGap) {
        lastStepTime = now;

    bool hrOK = true; //optional hr validation
    if (hrValidationEnabled && hrValidator) {
        hrValidator->update();     
        hrOK = hrValidator->isActiveState();
    }
    if (!hrOK) return;  // reject if HR inactive
        // Shift step window
        for (int i = 1; i < CADENCE_WINDOW; i++)
            stepTimes[i - 1] = stepTimes[i];
        stepTimes[CADENCE_WINDOW - 1] = now;

       

        computeCadenceAndVariance();

        if (currentVariance > 80000) return;  // reject irregular pattern

        // --- Walking entry with cadence stability ---
        
        if(!walking) {
            if (currentCadence >= 35 && currentCadence <= 120)
                bufferedSteps++;
            else
                bufferedSteps = 0;

            // Require consistent cadence stability
            static float lastCadence = 0;
            if (fabs(currentCadence - lastCadence) > 10)
                bufferedSteps = 0;

            lastCadence = currentCadence;

            if (bufferedSteps >= 5) {
                walking = true;
                stepCount += bufferedSteps;
                bufferedSteps = 0;
            }
        } else {
            stepCount++;
        }
    
    
    // --- Walking exit logic ---
    if (walking) {
        if ((now - lastStepTime) > 2500) {
            walking = false;
            lowCadenceFrames = 0;
        } else {
            computeCadenceAndVariance();
            if (currentCadence < 35 || currentCadence > 150)
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
    return currentCadence;
}

float StepCounter::getVariance() const {
    return currentVariance;
}

bool StepCounter::isWalking() const {
    return walking;
}
