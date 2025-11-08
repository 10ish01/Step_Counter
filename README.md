#  Step_Counter — Modular IMU-Based Step Detection (with Optional Heart-Rate Validation)

**Step_Counter** is a lightweight, modular step-counting library for ESP32 / Arduino.  
It detects steps using accelerometer + gyroscope, filters false steps, and optionally validates steps using a heart-rate sensor.

It is designed to work **standalone** or on top of the [MotionSensor](https://github.com/10ish01/MotionSensor) library, making it easy to integrate into existing projects.

---

## Features

- IMU-based step detection (accelerometer + gyroscope)  
- Filters out false steps caused by shakes or random hand movements  
- Optional heart-rate validation (MAX30102 or any other HR sensor)  
- Works standalone or with MotionSensor library  
- Easy to integrate, configure, and extend  

---

## Repository Structure


## Features

- IMU-based step detection (accelerometer + gyroscope)  
- Filters out false steps caused by shakes or random hand movements  
- Optional heart-rate validation (MAX30102 or any other HR sensor)  
- Works standalone or with MotionSensor library  
- Easy to integrate, configure, and extend  

---

## Repository Structure

/StepCounter/
├── StepCounter.h/.cpp # Core step counter logic
├── HRValidator.h # Abstract interface for HR validation
├── MAX30102Validator.cpp # Example HR validator
└── examples/
├── basic_step_counter.ino # Standalone IMU example



## How It Works

Step_Counter can work **two ways**:

1. **Standalone** — Directly reads IMU values from your device.  
2. **With MotionSensor library** — Leverages MotionSensor’s accelerometer & gyroscope driver for easier integration.


1. **Acceleration Magnitude**: Detects steps when magnitude exceeds threshold (~1.15 g).  
   `mag = sqrt(ax^2 + ay^2 + az^2)`

2. **Gyroscope Peak Filtering**: Ignores shakes (`gyroMag > 150 °/s`).  
   `gyroMag = sqrt(gx^2 + gy^2 + gz^2)`

3. **Minimum Time Gap**: Avoids double-counting (~300 ms between steps).

4. **Optional Heart-Rate Validation**: Steps counted only if HR rises above baseline (+10 BPM).

---

## Quick Start

### 🔹 Standalone Step Counter

```cpp
#include "StepCounter.h"

StepCounter stepCounter;

void setup() {
  Serial.begin(115200);
  stepCounter.begin();
}

void loop() {
  float ax, ay, az, gx, gy, gz;
  // Replace with your IMU readings
  stepCounter.update(ax, ay, az, gx, gy, gz);
  Serial.println(stepCounter.getStepCount());
  delay(50);
}
🔹 Using with MotionSensor
cpp
Copy code
#include "MotionSensor.h"
#include "StepCounter.h"

MotionSensor mpu;
StepCounter stepCounter(&mpu);

void setup() {
  Serial.begin(115200);
  mpu.begin();
  stepCounter.begin();
}

void loop() {
  mpu.update();
  stepCounter.update(
      mpu.getAccelX(), mpu.getAccelY(), mpu.getAccelZ(),
      mpu.getGyroX(),  mpu.getGyroY(),  mpu.getGyroZ()
  );
  Serial.println(stepCounter.getStepCount());
  delay(50);
}


🔹 Enabling Heart-Rate Validation
cpp
Copy code
#include "MAX30102Validator.cpp"

MAX30102Validator hr;
StepCounter stepCounter;

void setup() {
  hr.begin();
  stepCounter.begin();
  stepCounter.enableHRValidation(&hr);
}

void loop() {
  hr.update();
  stepCounter.update(ax, ay, az, gx, gy, gz);
  Serial.println(stepCounter.getStepCount());
}
```

### Configuration / Tuning
Parameter	Default   Description
accelThresh	1.15 g	  Minimum acceleration magnitude for a step
gyroPeak	150 °/s	  Max gyro magnitude to reject shakes
stepGap	    300 ms	  Minimum time between consecutive steps
HR delta	+10 BPM	  Heart-rate rise above baseline for confirmation

# Integration Philosophy
StepCounter — Core step detection algorithm

MotionSensor — Optional accelerometer & gyro driver

HRValidator — Abstract interface for optional HR modules

MAX30102Validator — Example HRValidator

All modules are independent; system works even without HR or MotionSensor.

# Recommended Usage Flow
Feed raw IMU data into StepCounter.

Optionally enable HRValidator.

Call getStepCount() to retrieve steps.

Optional: extend with stride length, activity detection, or distance estimation.

### Future Enhancements
Adaptive thresholding

Activity classification: walking / running / idle

Stride length estimation

Power-efficient sensor batching


### Notes
Modular and beginner-friendly

HR validation is optional and off by default

Compatible with other HR sensors with minimal modifications