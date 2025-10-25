#pragma once
#include <stdint.h>

// Mode kiểu Arduino (tự định nghĩa)
enum PinMode_custom : uint8_t {
    PIN_INPUT = 0,
    PIN_OUTPUT = 1,
    PIN_INPUT_PULLUP = 2,
    PIN_INPUT_PULLDOWN = 3
};

// API hậu tố _custom để không đụng Arduino gốc
void pinMode_custom(uint8_t pin, uint8_t mode);
void digitalWrite_custom(uint8_t pin, uint8_t value);
int  digitalRead_custom(uint8_t pin);
void digitalToggle_custom(uint8_t pin);

// Tiện ích
inline void digitalSet_custom(uint8_t pin)   { digitalWrite_custom(pin, 1); }
inline void digitalClear_custom(uint8_t pin) { digitalWrite_custom(pin, 0); }
