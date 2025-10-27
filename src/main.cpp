// HIGHT = Mưa/Tối 
// LOW   = Không Mưa / Sáng
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "gpio_driver.h"
#include "serial_driver.h"
#include "delay_driver.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
const uint8_t LDR_DO = 27;
const uint8_t RAIN_DO = 14;
const uint8_t MOTOR_PIN = 2;

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
    
    // Test: Bật motor 2 giây
    Serial.println("\n>>> TEST: Turning ON motor for 2 seconds...");
    digitalWrite_custom(MOTOR_PIN, 1);
    delay_custom(2000);
    digitalWrite_custom(MOTOR_PIN, 0);
    Serial.println(">>> Motor OFF\n");

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
    Serial.print(valueLDR == HIGH ? "HIGH" : "LOW");
    Serial.print(") | RAIN=");
    Serial.print(valueRAIN);
    Serial.print(" (");
    Serial.print(valueRAIN == HIGH ? "HIGH" : "LOW");
    Serial.print(") | ");

    // Logic điều khiển motor
    if (valueLDR == HIGH || valueRAIN == HIGH)
    {
        digitalWrite_custom(MOTOR_PIN, HIGH);
        Serial.println("=> MOTOR ON");
    }
    else
    {
        digitalWrite_custom(MOTOR_PIN, LOW);
        Serial.println("=> MOTOR OFF");
    }

    // Hiển thị LCD
    lcd.setCursor(5, 0);
    lcd.print("     "); // Xóa dữ liệu cũ
    lcd.setCursor(5, 0);
    lcd.print(valueLDR == HIGH ? "HIGH" : "LOW");

    lcd.setCursor(5, 1);
    lcd.print("     "); // Xóa dữ liệu cũ
    lcd.setCursor(5, 1);
    lcd.print(valueRAIN == HIGH ? "HIGH" : "LOW");

    delay_custom(500);
}
