#include "delay_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"

void delay_custom(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void delayMicroseconds_custom(uint32_t us) {
    esp_rom_delay_us(us);
}