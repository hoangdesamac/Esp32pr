#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "gpio_driver.h"
#include "serial_driver.h"
#include "delay_driver.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
const uint8_t LDR_DO = 27;
const uint8_t RAIN_DO = 14;

void setup() {
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("LDR Value:");
    lcd.setCursor(0, 1);
    lcd.print("Rain Value: ");
    pinMode_custom(LDR_DO, INPUT); // hoặc INPUT_PULLUP/INPUT_PULLDOWN tùy mạch
    pinMode_custom(RAIN_DO, INPUT);
}

void loop() {
    int valueLDR = digitalRead_custom(LDR_DO);
    int valueRAIN = digitalRead_custom(RAIN_DO);
    lcd.setCursor(11,0);
    lcd.print(valueLDR);
    lcd.setCursor(11, 1);
    lcd.print(valueRAIN);
    delay_custom(500);
}