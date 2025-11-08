#include <Wire.h>
#include "StepCounter.h"   // Your standalone step counter header

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
  Wire.write(0x00); // ±2g
  Wire.endTransmission();

  Wire.beginTransmission(MPU6886_ADDR);
  Wire.write(0x1B);
  Wire.write(0x00); // ±250 dps
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