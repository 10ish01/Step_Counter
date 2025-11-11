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
      intervalFilled(false),
      stepBufferIndex(0),
      totalStepsBuffer(0),
      walkingConfirmed(false)
{
    for (int i = 0; i < CADENCE_WINDOW; i++) stepIntervals[i] = 0;
    for (int i = 0; i < CADENCE_WINDOW; i++) stepTimes[i] = 0;
}

void StepCounter::begin() {
    stepCount = 0;
    currentState = IDLE;
    intervalIndex = 0;
    intervalFilled = false;
    stepBufferIndex = 0;
    totalStepsBuffer = 0;
    walkingConfirmed = false;
}

void StepCounter::update(float ax, float ay, float az, float gx, float gy, float gz) {
    bool isStep = detectStep(ax, ay, az, gx, gy, gz);
    unsigned long now = millis();

    // --- Update cadence tracking if a step is detected ---
if (isStep && (now - lastStepTime > stepGap )) {
    unsigned long interval = now - lastStepTime;
    lastStepTime = now;
  

    stepIntervals[intervalIndex] = interval;
    intervalIndex = (intervalIndex + 1) % CADENCE_WINDOW;
    if (intervalIndex == 0) intervalFilled = true;

    if (currentState == IDLE) checkCadence(now);

    else if (currentState == WALKING) {
            stepCount++;
        }
}


    switch (currentState) {
        case IDLE:
            
            if (checkCadence(now)) {
                setState(WALKING);
                // retroactively add buffered steps
                stepCount += CADENCE_WINDOW;
            }
            break;

        case WALKING:
      
            if ((now - lastStepTime) > 2000) {  // 2s inactivity → IDLE
                setState(IDLE);
                walkingConfirmed = false;
                totalStepsBuffer = 0;
            }
            break;
    }

    if (isStep) lastActiveTime = now;
}

// --- Step Detection ---
bool StepCounter::detectStep(float ax, float ay, float az, float gx, float gy, float gz) {
    float aMag = sqrt(ax * ax + ay * ay + az * az);
    float gMag = sqrt(gx * gx + gy * gy + gz * gz);
    return (aMag > accelThresh && gMag < gyroThresh);
}

// --- Cadence-based walking detection ---
bool StepCounter::checkCadence(unsigned long now) {
    stepTimes[stepBufferIndex] = now;
    stepBufferIndex = (stepBufferIndex + 1) % CADENCE_WINDOW;
    if (totalStepsBuffer < CADENCE_WINDOW) totalStepsBuffer++;

    if (totalStepsBuffer < CADENCE_WINDOW) return false;

    float totalDelta = 0;
    for (int i = 1; i < CADENCE_WINDOW; i++) {
        totalDelta += (stepTimes[i] - stepTimes[i - 1]);
    }
    float avgDelta = totalDelta / (CADENCE_WINDOW - 1);

    bool cadenceOK = (avgDelta > 350 && avgDelta < 1800);

    if (cadenceOK && !walkingConfirmed) {
        walkingConfirmed = true;
        return true;
    }
    return walkingConfirmed;
}

// --- State Setter ---
void StepCounter::setState(State newState) {
    if (newState != currentState)
        currentState = newState;
}

// --- Accessors ---
uint32_t StepCounter::getStepCount() {
    return stepCount;
}

StepCounter::State StepCounter::getCurrentState() const {
    return currentState;
}

// --- Cadence Getter ---
float StepCounter::getCadence() const {
    if (currentState == IDLE) return 0.0f; 

    int count = intervalFilled ? CADENCE_WINDOW : intervalIndex;
    if (count == 0) return 0.0f;

    unsigned long sum = 0;
    for (int i = 0; i < count; i++) sum += stepIntervals[i];
    float avgIntervalMs = (float)sum / count;
    return (avgIntervalMs > 0) ? (60000.0f / avgIntervalMs) : 0.0f;
}
