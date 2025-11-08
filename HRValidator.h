#pragma once

class HRValidator {
public:
    virtual void begin() = 0;
    virtual void update() = 0;
    virtual bool isActiveState() = 0; // returns true if HR rise indicates physical activity
    virtual ~HRValidator() {}
};
