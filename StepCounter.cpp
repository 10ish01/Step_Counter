#include "StepCounter.h"
#include <math.h>

StepCounter::StepCounter() {}
StepCounter::StepCounter(MotionSensor *sensor) : imu(sensor) {}

void StepCounter::begin(int resetPin) {
    EEPROM.begin(32);
    resetButtonPin = resetPin;

    if (resetButtonPin != -1) {
        pinMode(resetButtonPin, INPUT_PULLUP);
        unsigned long start = millis();
        while (millis() - start < 3000) {  // 3 sec window
            if (digitalRead(resetButtonPin) == LOW) {
                Serial.println("Threshold reset triggered!");
                resetThresholdsToDefault();
                break;
            }
            delay(50);
        }
    }

    loadThresholdsFromEEPROM();
    stepCount = 0;
    lastStepTime = 0;
}

void StepCounter::enableHRValidation(HRValidator *validator) {
    hrValidator = validator;
    hrValidationEnabled = true;
}

void StepCounter::disableHRValidation() {
    hrValidator = nullptr;
    hrValidationEnabled = false;
}

void StepCounter::update(float ax, float ay, float az, float gx, float gy, float gz) {
    float accelMag = sqrt(ax * ax + ay * ay + az * az);
    float gyroMag = sqrt(gx * gx + gy * gy + gz * gz);

    if (detectStep(accelMag, gyroMag)) {
        bool hrOK = true;
        if (hrValidationEnabled && hrValidator) {
            hrOK = hrValidator->isActiveState();
        }
        if (hrOK) stepCount++;
    }
}

bool StepCounter::detectStep(float accelMag, float gyroMag) {
    unsigned long now = millis();

    bool stepDetected = (accelMag > accelThresh && gyroMag < gyroPeak);
    if (stepDetected && (now - lastStepTime) > 300) {
        lastStepTime = now;
        return true;
    }
    return false;
}

int StepCounter::getStepCount() {
    return stepCount;
}

void StepCounter::setAccelThreshold(float threshold) {
    if (threshold > 0.5 && threshold < 3.0) {
        accelThresh = threshold;
        saveThresholdsToEEPROM();
    }
}

void StepCounter::setGyroPeak(float peak) {
    if (peak > 50 && peak < 500) {
        gyroPeak = peak;
        saveThresholdsToEEPROM();
    }
}

float StepCounter::getAccelThreshold() const { return accelThresh; }
float StepCounter::getGyroPeak() const { return gyroPeak; }

void StepCounter::saveThresholdsToEEPROM() {
    int addr = EEPROM_START_ADDR;
    EEPROM.put(addr, accelThresh);
    addr += sizeof(float);
    EEPROM.put(addr, gyroPeak);
    EEPROM.commit();
}

void StepCounter::loadThresholdsFromEEPROM() {
    int addr = EEPROM_START_ADDR;
    float a, g;
    EEPROM.get(addr, a);
    addr += sizeof(float);
    EEPROM.get(addr, g);

    if (a > 0.5 && a < 3.0 && g > 50 && g < 500) {
        accelThresh = a;
        gyroPeak = g;
    } else {
        resetThresholdsToDefault();
    }
}

void StepCounter::resetThresholdsToDefault(bool save) {
    accelThresh = 1.15;
    gyroPeak = 150.0;
    if (save) saveThresholdsToEEPROM();
    
}
