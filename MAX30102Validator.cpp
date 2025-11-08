#include "HRValidator.h"
#include <MAX30105.h>

class MAX30102Validator : public HRValidator {
public:
    void begin() override {
        sensor.begin(Wire, I2C_SPEED_STANDARD);
        baselineHR = 0;
        currentHR = 0;
    }

    void update() override {
        long irValue = sensor.getIR();
        if (irValue > 50000) {
            float bpm = computeBPM(irValue);
            currentHR = bpm;
        }
    }

    bool isActiveState() override {
        if (baselineHR == 0) baselineHR = currentHR;
        return (currentHR - baselineHR) > 10; // Active if HR rises 10 BPM
    }

private:
    MAX30105 sensor;
    float baselineHR;
    float currentHR;

    float computeBPM(long ir) {
        // Placeholder for HR algorithm
        return (ir % 100) + 60;
    }
};
