#  Step_Counter — Modular IMU-Based Step Detection (with Optional Heart-Rate Validation)

**Step_Counter** is a lightweight, modular step-counting library for ESP32 / Arduino.  
It detects steps using accelerometer + gyroscope, filters false steps, and optionally validates steps using a heart-rate sensor.

It is designed to work **standalone** or on top of the [MotionSensor](https://github.com/10ish01/MotionControl-ESP) library, making it easy to integrate into existing projects.

---



## Features

- IMU-based step detection (accelerometer + gyroscope)    
- Filters out false steps caused by shakes or random hand movements    
- Optional heart-rate validation (MAX30102 or any other HR sensor)    
- Works standalone or with MotionSensor library    
- Easy to integrate, configure, and extend    

---

## Repository Structure

StepCounter/  
├── StepCounter.h/.cpp # Core step counter logic  
├── HRValidator.h # Abstract interface for HR validation  
├── MAX30102Validator.cpp # Example HR validator  
├── examples/  
├── utils/  
     └── EEPROMManager.h/.cpp   # optional EEPROM utility


## How It Works

Step_Counter can work **two ways**:

1. **Standalone** — Directly reads IMU values from your device.    
2. **With MotionSensor library** — Leverages MotionSensor’s accelerometer & gyroscope driver for easier integration.  
  
### Flow  

1. **Acceleration + Gyroscope Filtering**-Each frame computes total acceleration and rotation magnitudes:  

aMag = sqrt(ax² + ay² + az²)   
gMag = sqrt(gx² + gy² + gz²)  
deltaA = |aMag - lastAMag|  
deltaG = |gMag - lastGMag|    

The thresholds adapt depending on whether the system is in entry or walking mode.  

If both exceed limits → flagged as spike (rejected).  

If only acceleration crosses threshold while gyro stays low → potential step.  
  
2. **Spike Rejection + Penalty System**- Consecutive motion spikes (high accel + gyro) increase a penalty counter.  
While penalties are active, thresholds tighten and the minimum step gap increases slightly.  
Penalties decay automatically after ~3 seconds of stability.  
This prevents false positives from vigorous shakes or abrupt movements.    
  
3. **Adaptive Step Validation**- When a raw step candidate passes spike and gap checks, it’s accepted only if:  
-The time since the previous step is within a valid range (≈250–2500 ms).  
-The variance of recent step intervals stays low (to ensure rhythm consistency).  
-Outlier or irregular intervals are ignored.    
Optional HR validation: if enabled, the step is accepted only when the HR validator indicates active physical activity.  
  
4. **Walking Entry Logic**- In IDLE state, the algorithm temporarily buffers potential steps.  
If cadence from the recent buffer lies in the walking zone (≈35–120 SPM) for several consecutive detections (~5 steps), walking is confirmed.  
The buffered steps are then committed, and the state transitions to WALKING.  
  
5. **Walking Exit Logic (Inactivity or Drift)**- If no steps occur for > 2.5 s → return to IDLE.  
If cadence drops below 35 SPM or exceeds 150 SPM for multiple frames → exit WALKING.  
This ensures stability and quick recovery after pauses.  

6. **Live Cadence Computation**- During walking, cadence is recalculated from recent valid step intervals (≈ 4–6 s window).  
Outliers are filtered, and a low-pass filter smooths the cadence:  
smoothCadence = 0.6 * instantCadence + 0.4 * previousCadence    
  
Provides a responsive yet stable step rate estimate in real time.   
---

## Quick Start

## 🔹 Standalone Step Counter (basis MPU6886 - you can modify to your imu easily)
Connect your IMU to default SDA and SCL pins for your board (21 and 22 for ESP32, respectively)
```cpp
#include <Wire.h>
#include "StepCounter.h"   

#define MPU6886_ADDR 0x68  // I2C address

StepCounter stepCounter;   // Create instance

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // --- Initialize MPU6886 ---
  Wire.beginTransmission(MPU6886_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00); // Wake up
  Wire.endTransmission();

  Wire.beginTransmission(MPU6886_ADDR);
  Wire.write(0x1C);
  Wire.write(0x00); 
  Wire.endTransmission();

  Wire.beginTransmission(MPU6886_ADDR);
  Wire.write(0x1B);
  Wire.write(0x00); 
  Wire.endTransmission();

  Serial.println("MPU6886 initialized!");
  stepCounter.begin();   // init internal buffers etc
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz, tempRaw;

  // --- Read raw sensor data ---
  Wire.beginTransmission(MPU6886_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6886_ADDR, 14, true);

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
  tempRaw = Wire.read() << 8 | Wire.read();
  gx = Wire.read() << 8 | Wire.read();
  gy = Wire.read() << 8 | Wire.read();
  gz = Wire.read() << 8 | Wire.read();

  // --- Convert to physical units ---
  float ax_g = ax / 16384.0;   // accel in g
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;
  float gx_dps = gx / 131.0;   // gyro in °/s
  float gy_dps = gy / 131.0;
  float gz_dps = gz / 131.0;

  // --- Feed data to StepCounter ---
  stepCounter.update(ax_g, ay_g, az_g, gx_dps, gy_dps, gz_dps);

  // --- Print results ---
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) { // print every 1s
    Serial.print("Steps: ");
    Serial.println(stepCounter.getStepCount());
    lastPrint = millis();
  }

  delay(20);  // ~50 Hz
}
```


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


## Optional Utilities

### EEPROMManager
A standalone utility for managing persistent storage of step history (day-wise)

Sync step data externally at least once every 7 days to avoid losing records

You can adapt this code or integrate it into your existing project for EEPROM management.

Features:   
-Store daily step counts in a rolling history (configurable up to 7 days).

-Maintain a current step count that updates at the end of the day.

-Track day pointer to know which day in the history is active.

-Flexible API for reading, updating, and syncing step data.

-Centralized EEPROM management to avoid conflicts in larger projects.

Example Usage 
```cpp
#include <Arduino.h>
#include "EEPROMManager.h"

EEPROMManager eeprom;

void setup() {
  Serial.begin(115200);

  // Initialize EEPROM manager
  eeprom.begin();

  // Example: Load current step count from EEPROM
  uint16_t todaySteps = eeprom.loadStepCount();
  Serial.print("Current steps today: ");
  Serial.println(todaySteps);

  // Example: Increment step count
  todaySteps += 123; // Suppose you counted 123 new steps
  eeprom.saveStepCount(todaySteps);

  // Example: Finalize the day (store today's steps in history, advance day pointer)
  eeprom.finalizeDay(todaySteps);

  // Example: Retrieve last 7 days of step history
  uint16_t history[7];
  eeprom.getStepHistory(history);
  Serial.println("Last 7 days of steps:");
  for (int i = 0; i < 7; i++) {
    Serial.print("Day ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(history[i]);
  }

  // Example: Clear step history
  // eeprom.clearStepHistory();
}

void loop() {
  // Your main step counting code here
}
```

(This library **is not required** for Step_Counter to work.  
It can be used if you want centralized EEPROM management across your project.)


### Future Enhancements
Enhanced Activity classification: driving / walking / running / idle

Stride length estimation

Power-efficient sensor batching

Sleep tracking

### Notes
Modular and beginner-friendly

HR validation is optional and off by default

Compatible with other HR sensors with minimal modifications
