#include <Arduino.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
const int ldrDO = 27;
void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
}

void loop() {
  int value = digitalRead(ldrDO);
  lcd.print("LDR Value:");
  lcd.setCursor(10, 0);
  lcd.print(value);
  delay(500);
}
