#include <Wire.h>
#include "StepCounter.h"
#include "EEPROMManager.h"

#define MPU6886_ADDR 0x68  // I2C address

StepCounter stepCounter;
EEPROMManager eeprom;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize MPU6886
  Wire.beginTransmission(MPU6886_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
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

  // Initialize StepCounter
  stepCounter.begin();

  // Initialize EEPROM manager
  eeprom.begin();

  // Load today's steps from EEPROM and set in StepCounter
  uint16_t todaySteps = eeprom.loadStepCount();
  Serial.print("Starting today with steps: ");
  Serial.println(todaySteps);
  // If you want, you could set stepCounter.stepCount = todaySteps via a setter (optional)
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz, tempRaw;

  // Read raw IMU data
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

  float ax_g = ax / 16384.0;
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;
  float gx_dps = gx / 131.0;
  float gy_dps = gy / 131.0;
  float gz_dps = gz / 131.0;

  // Update StepCounter
  stepCounter.update(ax_g, ay_g, az_g, gx_dps, gy_dps, gz_dps);

  // Periodically save today's step count to EEPROM (e.g., every minute)
  static unsigned long lastSave = 0;
  if (millis() - lastSave > 60000) { // every 60s
    eeprom.saveStepCount(stepCounter.getStepCount());
    lastSave = millis();
  }

  // Optional: print steps every second
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    Serial.print("Steps today: ");
    Serial.println(stepCounter.getStepCount());
    lastPrint = millis();
  }

  delay(20);
}
