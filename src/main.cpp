// HIGHT = Không Mưa/Tối 
// LOW   = Mưa / Sáng
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "gpio_driver.h"
#include "serial_driver.h"
#include "delay_driver.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
const uint8_t LDR_DO = 27;
const uint8_t RAIN_DO = 14;
// ===== Motor Control via DRV8833 =====
const uint8_t MOTOR_IN1 = 18;
const uint8_t MOTOR_IN2 = 19;
const uint8_t MOTOR_PIN = 2;
// === MOTOR CONTROL FUNCTIONS ===
void motorStop() {
    digitalWrite_custom(MOTOR_IN1, LOW);
    digitalWrite_custom(MOTOR_IN2, LOW);
}

void motorForward() {
    digitalWrite_custom(MOTOR_IN1, HIGH);
    digitalWrite_custom(MOTOR_IN2, LOW);
}

void motorReverse() {
    digitalWrite_custom(MOTOR_IN1, LOW);
    digitalWrite_custom(MOTOR_IN2, HIGH);
}

void setup()
{
    Serial.begin(9600);
    delay(500); // Đợi Serial khởi động
    
    Serial.println("\n\n=== SYSTEM STARTING ===");
    
    Wire.begin(21, 22);
    
    // LCD khởi tạo
    lcd.init();
    lcd.backlight(); // Bật backlight
    lcd.setCursor(0, 0);
    lcd.print("Initializing...");
    
    // Cấu hình chân I/O
    Serial.println("Configuring GPIO pins...");
    pinMode_custom(LDR_DO, PIN_INPUT);
    Serial.println("LDR_DO (GPIO 27) set as INPUT");
    
    pinMode_custom(RAIN_DO, PIN_INPUT);
    Serial.println("RAIN_DO (GPIO 14) set as INPUT");
    
    pinMode_custom(MOTOR_PIN, PIN_OUTPUT);
    Serial.println("MOTOR_PIN (GPIO 2) set as OUTPUT");

    pinMode_custom(MOTOR_IN1, PIN_OUTPUT);
    pinMode_custom(MOTOR_IN2, PIN_OUTPUT);
    Serial.println("MOTOR_IN1 (GPIO 18) and MOTOR_IN2 (GPIO 19) set as OUTPUT");

    
        // Test: Quay motor 2 giây tiến, 2 giây lùi
    Serial.println("\n>>> TEST: Motor forward 2s...");
    motorForward();
    delay_custom(2000);

    Serial.println(">>> TEST: Motor reverse 2s...");
    motorReverse();
    delay_custom(2000);

    motorStop();
    Serial.println(">>> TEST: Motor stopped\n");


    // LCD khởi tạo lại
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LDR:     ");   // Thêm khoảng trắng để xóa dữ liệu cũ
    lcd.setCursor(0, 1);
    lcd.print("RAIN:    "); 
    Serial.println("=== SETUP COMPLETE ===\n");
}

void loop()
{
    int valueLDR = digitalRead_custom(LDR_DO);
    int valueRAIN = digitalRead_custom(RAIN_DO);

    // In debug thông tin
    Serial.print("LDR=");
    Serial.print(valueLDR);
    Serial.print(" (");
    Serial.print(valueLDR == HIGH ? "DARK" : "LIGHT"); // Sửa lại để dễ hiểu hơn
    Serial.print(") | RAIN=");
    Serial.print(valueRAIN);
    Serial.print(" (");
    Serial.print(valueRAIN == LOW ? "RAIN" : "NO RAIN"); // Sửa lại để dễ hiểu hơn
    Serial.print(") | ");

        // Điều khiển motor qua DRV8833
    if (valueLDR == HIGH || valueRAIN == LOW)
    {
        motorReverse(); // Thu đồ
        Serial.println("=> MOTOR: THU (reverse)");
    }
    else
    {
        motorForward(); // Phơi đồ
        Serial.println("=> MOTOR: PHƠI (forward)");
    }

    lcd.setCursor(5, 0);
    // Dựa theo comment: HIGH là Tối, LOW là Sáng
    lcd.print(valueLDR == HIGH ? "DARK" : "LIGHT");

    lcd.setCursor(5, 1);
    // Dựa theo comment: LOW là Mưa
    lcd.print(valueRAIN == LOW ? "RAIN" : "NO RAIN");

    delay_custom(500);
}