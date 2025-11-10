#include "EEPROMManager.h"

// --- Initialization ---
void EEPROMManager::begin() {
    EEPROM.begin(EEPROM_SIZE);
    currentDayIndex = EEPROM.read(ADDR_DAY_POINTER);
    if (currentDayIndex > 6) currentDayIndex = 0;
}

// ---------- STEP DATA ----------
void EEPROMManager::saveStepCount(uint16_t steps) {
    EEPROM.put(ADDR_STEP_COUNT, steps);
    EEPROM.commit();
}

uint16_t EEPROMManager::loadStepCount() {
    uint16_t steps;
    EEPROM.get(ADDR_STEP_COUNT, steps);
    if (steps > 100000) steps = 0;
    return steps;
}

void EEPROMManager::finalizeDay(uint16_t todaySteps) {
    int addr = ADDR_STEP_HISTORY + (currentDayIndex * sizeof(uint16_t));
    EEPROM.put(addr, todaySteps);

    currentDayIndex = (currentDayIndex + 1) % 7;
    EEPROM.write(ADDR_DAY_POINTER, currentDayIndex);

    uint16_t resetVal = 0;
    EEPROM.put(ADDR_STEP_COUNT, resetVal);
    EEPROM.commit();
}

void EEPROMManager::getStepHistory(uint16_t history[7]) {
    for (int i = 0; i < 7; i++) {
        int addr = ADDR_STEP_HISTORY + (i * sizeof(uint16_t));
        EEPROM.get(addr, history[i]);
    }
}

void EEPROMManager::clearStepHistory() {
    for (int i = 0; i < 7; i++) {
        int addr = ADDR_STEP_HISTORY + (i * sizeof(uint16_t));
        uint16_t zero = 0;
        EEPROM.put(addr, zero);
    }
    currentDayIndex = 0;
    EEPROM.write(ADDR_DAY_POINTER, currentDayIndex);
    EEPROM.commit();
}

uint8_t EEPROMManager::getCurrentDayIndex() {
    return currentDayIndex;
}

// ---------- DEVICE SETTINGS ----------
void EEPROMManager::saveDeviceSetting(uint8_t index, uint8_t value) {
    if (index < DEVICE_SECTION_SIZE)
        EEPROM.write(DEVICE_SECTION_START + index, value);
    EEPROM.commit();
}

uint8_t EEPROMManager::loadDeviceSetting(uint8_t index) {
    if (index < DEVICE_SECTION_SIZE)
        return EEPROM.read(DEVICE_SECTION_START + index);
    return 0xFF;
}

// ---------- BLE SETTINGS ----------
void EEPROMManager::saveBLESetting(uint8_t index, uint8_t value) {
    if (index < BLE_SECTION_SIZE)
        EEPROM.write(BLE_SECTION_START + index, value);
    EEPROM.commit();
}

uint8_t EEPROMManager::loadBLESetting(uint8_t index) {
    if (index < BLE_SECTION_SIZE)
        return EEPROM.read(BLE_SECTION_START + index);
    return 0xFF;
}

// ---------- MISC ----------
void EEPROMManager::saveMiscData(uint8_t index, uint8_t value) {
    if (index < MISC_SECTION_SIZE)
        EEPROM.write(MISC_SECTION_START + index, value);
    EEPROM.commit();
}

uint8_t EEPROMManager::loadMiscData(uint8_t index) {
    if (index < MISC_SECTION_SIZE)
        return EEPROM.read(MISC_SECTION_START + index);
    return 0xFF;
}
