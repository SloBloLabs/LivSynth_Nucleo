#pragma once

#include <cstdint>

class DacInternal {
public:
    void init();

    inline uint32_t getValue() { return _value; };
    void setValue(uint32_t value);

private:
    uint32_t _value;
};