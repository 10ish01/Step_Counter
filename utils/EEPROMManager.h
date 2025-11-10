#pragma once
#include <Arduino.h>
#include <EEPROM.h>

class EEPROMManager {
public:
    void begin();

    // ---------- STEP DATA ----------
    void saveStepCount(uint16_t steps);
    uint16_t loadStepCount();
    void finalizeDay(uint16_t todaySteps);
    void getStepHistory(uint16_t history[7]);
    void clearStepHistory();
    uint8_t getCurrentDayIndex();

    // ---------- DEVICE SETTINGS ----------
    void saveDeviceSetting(uint8_t index, uint8_t value);
    uint8_t loadDeviceSetting(uint8_t index);

    // ---------- BLE SETTINGS ----------
    void saveBLESetting(uint8_t index, uint8_t value);
    uint8_t loadBLESetting(uint8_t index);

    // ---------- MISC ----------
    void saveMiscData(uint8_t index, uint8_t value);
    uint8_t loadMiscData(uint8_t index);

private:
    // === GLOBAL LAYOUT ===
    static constexpr int EEPROM_SIZE = 32; //Update basis your requirements overall

    // Step data section (0–31)
    static constexpr int STEP_SECTION_START = 0;
    static constexpr int STEP_SECTION_SIZE = 32;



    // === Step Data Layout ===
    static constexpr int ADDR_STEP_COUNT   = STEP_SECTION_START;          // 2 bytes
    static constexpr int ADDR_DAY_POINTER  = STEP_SECTION_START + 2;      // 1 byte
    static constexpr int ADDR_STEP_HISTORY = STEP_SECTION_START + 4;      // 14 bytes (7 × 2)

    uint8_t currentDayIndex = 0;
};
