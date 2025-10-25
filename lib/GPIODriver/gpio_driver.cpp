#include "gpio_driver.h"
#include <stdint.h>

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP_PLATFORM)

#ifdef __cplusplus
extern "C" {
#endif
#include "soc/soc.h"
#include "soc/gpio_reg.h"
#include "soc/io_mux_reg.h"
#ifdef __cplusplus
}
#endif

#if defined(PIN_PULLDWN_EN) && !defined(PIN_PULLDOWN_EN)
#define PIN_PULLDOWN_EN  PIN_PULLDWN_EN
#endif
#if defined(PIN_PULLDWN_DIS) && !defined(PIN_PULLDOWN_DIS)
#define PIN_PULLDOWN_DIS PIN_PULLDWN_DIS
#endif
#ifndef PIN_PULLUP_EN
#define PIN_PULLUP_EN(reg)     REG_SET_BIT((reg), FUN_PU)
#endif
#ifndef PIN_PULLUP_DIS
#define PIN_PULLUP_DIS(reg)    REG_CLR_BIT((reg), FUN_PU)
#endif
#ifndef PIN_PULLDWN_EN
#define PIN_PULLDWN_EN(reg)    REG_SET_BIT((reg), FUN_PD)
#endif
#ifndef PIN_PULLDWN_DIS
#define PIN_PULLDWN_DIS(reg)   REG_CLR_BIT((reg), FUN_PD)
#endif
#ifndef PIN_INPUT_ENABLE
#define PIN_INPUT_ENABLE(reg)  REG_SET_BIT((reg), FUN_IE)
#endif
#ifndef PIN_INPUT_DISABLE
#define PIN_INPUT_DISABLE(reg) REG_CLR_BIT((reg), FUN_IE)
#endif

static inline uint32_t io_mux_reg_for_pin(uint8_t pin) {
    switch (pin) {
        case 0:  return IO_MUX_GPIO0_REG;
        case 1:  return IO_MUX_GPIO1_REG;
        case 2:  return IO_MUX_GPIO2_REG;
        case 3:  return IO_MUX_GPIO3_REG;
        case 4:  return IO_MUX_GPIO4_REG;
        case 5:  return IO_MUX_GPIO5_REG;
        case 12: return IO_MUX_GPIO12_REG;
        case 13: return IO_MUX_GPIO13_REG;
        case 14: return IO_MUX_GPIO14_REG;
        case 15: return IO_MUX_GPIO15_REG;
        case 16: return IO_MUX_GPIO16_REG;
        case 17: return IO_MUX_GPIO17_REG;
        case 18: return IO_MUX_GPIO18_REG;
        case 19: return IO_MUX_GPIO19_REG;
        case 21: return IO_MUX_GPIO21_REG;
        case 22: return IO_MUX_GPIO22_REG;
        case 23: return IO_MUX_GPIO23_REG;
        case 25: return IO_MUX_GPIO25_REG;
        case 26: return IO_MUX_GPIO26_REG;
        case 27: return IO_MUX_GPIO27_REG;
        case 32: return IO_MUX_GPIO32_REG;
        case 33: return IO_MUX_GPIO33_REG;
        case 34: return IO_MUX_GPIO34_REG;
        case 35: return IO_MUX_GPIO35_REG;
        case 36: return IO_MUX_GPIO36_REG;
        case 39: return IO_MUX_GPIO39_REG;
        default: return 0;
    }
}

static inline void setDirection(uint8_t pin, bool output) {
    if (pin <= 31) {
        if (output) REG_WRITE(GPIO_ENABLE_W1TS_REG, (1u << pin));
        else        REG_WRITE(GPIO_ENABLE_W1TC_REG, (1u << pin));
    }
}

static inline void setPullAndInput(uint8_t pin, uint8_t mode) {
    uint32_t reg = io_mux_reg_for_pin(pin);
    if (!reg) return;

#if defined(PIN_FUNC_SELECT) && defined(FUNC_GPIO)
    PIN_FUNC_SELECT(reg, FUNC_GPIO);
#endif

    PIN_PULLUP_DIS(reg);
    PIN_PULLDWN_DIS(reg);

    if (mode == PIN_INPUT || mode == PIN_INPUT_PULLUP || mode == PIN_INPUT_PULLDOWN) {
        PIN_INPUT_ENABLE(reg);
    }

    if (mode == PIN_INPUT_PULLUP) {
        PIN_PULLUP_EN(reg);
    } else if (mode == PIN_INPUT_PULLDOWN) {
        PIN_PULLDWN_EN(reg);
    }
}

void pinMode_custom(uint8_t pin, uint8_t mode) {
    setDirection(pin, mode == PIN_OUTPUT);
    setPullAndInput(pin, mode);
}

void digitalWrite_custom(uint8_t pin, uint8_t value) {
    if (pin > 31) return;
    REG_WRITE(value ? GPIO_OUT_W1TS_REG : GPIO_OUT_W1TC_REG, (1u << pin));
}

int digitalRead_custom(uint8_t pin) {
    if (pin <= 31) return (REG_READ(GPIO_IN_REG)  >> pin) & 1u;
    if (pin <= 39) return (REG_READ(GPIO_IN1_REG) >> (pin - 32)) & 1u;
    return 0;
}

void digitalToggle_custom(uint8_t pin) {
    if (pin > 31) return;
    uint32_t out = REG_READ(GPIO_OUT_REG);
    REG_WRITE(((out >> pin) & 1u) ? GPIO_OUT_W1TC_REG : GPIO_OUT_W1TS_REG, (1u << pin));
}

#else

// ========== Fallback bare-metal (không SDK) ==========
#define DR_REG_GPIO_BASE       0x3FF44000u
#define GPIO_OUT_REG           (DR_REG_GPIO_BASE + 0x04u)
#define GPIO_OUT_W1TS_REG      (DR_REG_GPIO_BASE + 0x08u)
#define GPIO_OUT_W1TC_REG      (DR_REG_GPIO_BASE + 0x0Cu)
#define GPIO_ENABLE_W1TS_REG   (DR_REG_GPIO_BASE + 0x24u)
#define GPIO_ENABLE_W1TC_REG   (DR_REG_GPIO_BASE + 0x28u)
#define GPIO_IN_REG            (DR_REG_GPIO_BASE + 0x3Cu)
#define GPIO_IN1_REG           (DR_REG_GPIO_BASE + 0x40u)

#define REG_WRITE(addr, val)   (*((volatile uint32_t *)(addr)) = (uint32_t)(val))
#define REG_READ(addr)         (*((volatile uint32_t *)(addr)))

void pinMode_custom(uint8_t pin, uint8_t mode) {
    if (pin <= 31) {
        if (mode == OUTPUT) REG_WRITE(GPIO_ENABLE_W1TS_REG, (1u << pin));
        else                REG_WRITE(GPIO_ENABLE_W1TC_REG, (1u << pin));
    }
    (void)mode; // Không xử lý pull ở bản tối giản
}

void digitalWrite_custom(uint8_t pin, uint8_t value) {
    if (pin > 31) return;
    REG_WRITE(value ? GPIO_OUT_W1TS_REG : GPIO_OUT_W1TC_REG, (1u << pin));
}

int digitalRead_custom(uint8_t pin) {
    if (pin <= 31) return (REG_READ(GPIO_IN_REG)  >> pin) & 1u;
    if (pin <= 39) return (REG_READ(GPIO_IN1_REG) >> (pin - 32)) & 1u;
    return 0;
}

void digitalToggle_custom(uint8_t pin) {
    if (pin > 31) return;
    uint32_t out = REG_READ(GPIO_OUT_REG);
    REG_WRITE(((out >> pin) & 1u) ? GPIO_OUT_W1TC_REG : GPIO_OUT_W1TS_REG, (1u << pin));
}
#endif