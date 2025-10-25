#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "gpio_driver.h"
#include "serial_driver.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
const uint8_t LDR_DO = 27;

void setup() {
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("LDR Value:");
  pinMode_custom(LDR_DO, INPUT); // hoặc INPUT_PULLUP/INPUT_PULLDOWN tùy mạch
}

void loop() {
    int value = digitalRead_custom(LDR_DO);
    lcd.setCursor(11,0);
    lcd.print(value);
    lcd.print(' ');
    delay(500);
}