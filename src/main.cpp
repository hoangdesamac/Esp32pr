#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "gpio_driver.h"
#include "serial_driver.h"
#include "delay_driver.h"

// ===== LCD =====
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== INPUT SENSOR =====
const uint8_t LDR_DO  = 26;   // Cảm biến ánh sáng (HIGH = tối, LOW = sáng)
const uint8_t RAIN_DO = 14;   // Cảm biến mưa (HIGH = khô, LOW = mưa)

// ===== LIMIT SWITCHES =====
const uint8_t LIMIT_SWITCH_START = 25; // Công tắc hành trình THU đồ
const uint8_t LIMIT_SWITCH_END   = 27; // Công tắc hành trình PHƠI đồ

// ===== MOTOR CONTROL via DRV8833 =====
const uint8_t MOTOR_IN1 = 18;
const uint8_t MOTOR_IN2 = 19;
const uint8_t MOTOR_PIN = 2;

// ===== Motor State Enum =====
enum MotorState {
    MOTOR_STOPPED,
    MOTOR_FORWARD, // PHƠI đồ
    MOTOR_REVERSE  // THU đồ
};

MotorState currentMotorState = MOTOR_STOPPED;

// ===== LATCH flags (ngăn motor tự chạy lại) =====
bool startLimitLatched = false;
bool endLimitLatched   = false;

// ===== MOTOR CONTROL =====
void motorStop() {
    digitalWrite_custom(MOTOR_IN1, LOW);
    digitalWrite_custom(MOTOR_IN2, LOW);
}

void motorForward() { // PHƠI đồ
    digitalWrite_custom(MOTOR_IN1, HIGH);
    digitalWrite_custom(MOTOR_IN2, LOW);
}

void motorReverse() { // THU đồ
    digitalWrite_custom(MOTOR_IN1, LOW);
    digitalWrite_custom(MOTOR_IN2, HIGH);
}

void setup() {
    Serial.begin(9600);
    delay(500);

    Wire.begin(21, 22);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Initializing...");

    // ===== Pin configuration =====
    pinMode_custom(LDR_DO, PIN_INPUT);
    pinMode_custom(RAIN_DO, PIN_INPUT);
    pinMode_custom(MOTOR_PIN, PIN_OUTPUT);
    pinMode_custom(MOTOR_IN1, PIN_OUTPUT);
    pinMode_custom(MOTOR_IN2, PIN_OUTPUT);

    pinMode(LIMIT_SWITCH_START, INPUT_PULLUP);
    pinMode(LIMIT_SWITCH_END, INPUT_PULLUP);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LDR: ");
    lcd.setCursor(0, 1);
    lcd.print("RAIN:");

    Serial.println("=== SETUP COMPLETE ===\n");
}

void loop() {
    // ===== Đọc cảm biến =====
    int valueLDR  = digitalRead_custom(LDR_DO);
    int valueRAIN = digitalRead_custom(RAIN_DO);

    int startPressed = digitalRead(LIMIT_SWITCH_START);
    int endPressed   = digitalRead(LIMIT_SWITCH_END);

    // ===== Debug =====
    Serial.print("LDR="); Serial.print(valueLDR);
    Serial.print(" | RAIN="); Serial.print(valueRAIN);
    Serial.print(" | S1="); Serial.print(startPressed ? "PRESSED" : "RELEASED");
    Serial.print(" | S2="); Serial.print(endPressed ? "PRESSED" : "RELEASED");
    Serial.print(" | ");

    // ===== Logic xác định hướng motor mong muốn =====
    MotorState desiredMotorState =
        (valueLDR == HIGH && valueRAIN == HIGH) ? MOTOR_FORWARD : MOTOR_REVERSE;

    // ===== Reset latch khi hướng điều khiển thay đổi =====
    // (chỉ reset latch của hướng ngược lại)
    if (desiredMotorState == MOTOR_FORWARD && currentMotorState != MOTOR_FORWARD) {
        startLimitLatched = false;
    }
    if (desiredMotorState == MOTOR_REVERSE && currentMotorState != MOTOR_REVERSE) {
        endLimitLatched = false;
    }

    // ===== Logic dừng motor theo latch =====
    if ((desiredMotorState == MOTOR_REVERSE && startPressed && !startLimitLatched) ||
        (desiredMotorState == MOTOR_FORWARD && endPressed && !endLimitLatched)) {
        motorStop();
        currentMotorState = MOTOR_STOPPED;

        if (desiredMotorState == MOTOR_REVERSE) startLimitLatched = true;
        if (desiredMotorState == MOTOR_FORWARD) endLimitLatched = true;

        Serial.println("=> MOTOR STOPPED (limit switch latch)");
    }
    else if (currentMotorState == MOTOR_STOPPED &&
             !startLimitLatched && !endLimitLatched) {
        // Motor chỉ chạy khi chưa latch
        if (desiredMotorState == MOTOR_FORWARD) motorForward();
        else motorReverse();
        currentMotorState = desiredMotorState;

        Serial.print("=> MOTOR START: ");
        Serial.println(desiredMotorState == MOTOR_FORWARD ? "PHOI (FORWARD)" : "THU (REVERSE)");
    }
    else if (desiredMotorState != currentMotorState &&
             !startLimitLatched && !endLimitLatched) {
        motorStop();
        delay_custom(500);
        if (desiredMotorState == MOTOR_FORWARD) motorForward();
        else motorReverse();
        currentMotorState = desiredMotorState;

        Serial.print("=> MOTOR CHANGE DIRECTION: ");
        Serial.println(desiredMotorState == MOTOR_FORWARD ? "PHOI (FORWARD)" : "THU (REVERSE)");
    }

    // ===== Cập nhật LCD =====
    lcd.setCursor(5, 0);
    lcd.print(valueLDR == HIGH ? "DARK " : "LIGHT");
    lcd.setCursor(5, 1);
    lcd.print(valueRAIN == LOW ? "RAIN " : "CLEAR");

    lcd.setCursor(12, 0);
    lcd.print(startPressed ? "STOP" : "OK  ");
    lcd.setCursor(12, 1);
    lcd.print(endPressed ? "STOP" : "OK  ");

    Serial.print("STATE=");
    if (currentMotorState == MOTOR_FORWARD) Serial.println("FORWARD");
    else if (currentMotorState == MOTOR_REVERSE) Serial.println("REVERSE");
    else Serial.println("STOPPED");

    delay_custom(300);
}
