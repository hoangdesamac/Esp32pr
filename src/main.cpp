#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "gpio_driver.h"
#include "serial_driver.h"
#include "delay_driver.h"

// ===== LCD =====
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== INPUT SENSOR =====
const uint8_t LDR_DO = 26;
const uint8_t RAIN_DO = 14;

// ===== LIMIT SWITCHES =====
const uint8_t LIMIT_SWITCH_START = 25;
const uint8_t LIMIT_SWITCH_END = 27;

// ===== MOTOR CONTROL DRV8833 =====
const uint8_t MOTOR_IN1 = 18;
const uint8_t MOTOR_IN2 = 19;
const uint8_t MOTOR_PIN = 2;

// ===== Motor State=====
enum MotorState
{
    MOTOR_STOPPED,
    MOTOR_FORWARD, // PHƠI đồ
    MOTOR_REVERSE  // THU đồ
};

MotorState currentMotorState = MOTOR_STOPPED;

// ===== LATCH flags =====
bool startLimitLatched = false;
bool endLimitLatched = false;

// ===== MOTOR CONTROL =====
void motorStop()
{
    digitalWrite_custom(MOTOR_IN1, LOW);
    digitalWrite_custom(MOTOR_IN2, LOW);
}

void motorForward()
{ // PHƠI đồ
    digitalWrite_custom(MOTOR_IN1, HIGH);
    digitalWrite_custom(MOTOR_IN2, LOW);
}

void motorReverse()
{ // THU đồ
    digitalWrite_custom(MOTOR_IN1, LOW);
    digitalWrite_custom(MOTOR_IN2, HIGH);
}

void setup()
{
    Serial.begin(9600);
    delay(500); // 0.5s

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

    // ===== LCD Shown =====
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LDR: ");
    lcd.setCursor(0, 1);
    lcd.print("RAIN:");

    Serial.println("=== SETUP COMPLETE ===\n");
}

void loop()
{
    // ===== Read Sensor =====
    int valueLDR = digitalRead_custom(LDR_DO);
    int valueRAIN = digitalRead_custom(RAIN_DO);
    // ===== Read Limit Switches =====
    int startPressed = digitalRead(LIMIT_SWITCH_START);
    int endPressed = digitalRead(LIMIT_SWITCH_END);

    // ===== Definite Motor Direction Logic =====
    MotorState desiredMotorState =
        (valueLDR == HIGH && valueRAIN == HIGH) ? MOTOR_FORWARD : MOTOR_REVERSE;

    // ===== Reset latch=====
    // (Reset Latch In Reverse Direction)
    if (desiredMotorState == MOTOR_FORWARD && currentMotorState != MOTOR_FORWARD)
    {
        startLimitLatched = false;
    }
    if (desiredMotorState == MOTOR_REVERSE && currentMotorState != MOTOR_REVERSE)
    {
        endLimitLatched = false;
    }

    // ===== Motor Stopped Logic For latch =====
    if ((desiredMotorState == MOTOR_REVERSE && startPressed && !startLimitLatched) ||
        (desiredMotorState == MOTOR_FORWARD && endPressed && !endLimitLatched))
    {
        motorStop();
        currentMotorState = MOTOR_STOPPED;

        if (desiredMotorState == MOTOR_REVERSE)
            startLimitLatched = true;
        if (desiredMotorState == MOTOR_FORWARD)
            endLimitLatched = true;
    }
    else if (currentMotorState == MOTOR_STOPPED &&
            !startLimitLatched && !endLimitLatched)
    {
        // Motor Run Without Latch
        if (desiredMotorState == MOTOR_FORWARD)
            motorForward();
        else
            motorReverse();
        currentMotorState = desiredMotorState;
    }
    else if (desiredMotorState != currentMotorState &&
            !startLimitLatched && !endLimitLatched)
    {
        motorStop();
        delay_custom(500); // 0.5s
        if (desiredMotorState == MOTOR_FORWARD)
            motorForward();
        else
            motorReverse();
        currentMotorState = desiredMotorState;
    }

    // ===== LCD Modify =====
    lcd.setCursor(5, 0);
    lcd.print(valueLDR == HIGH ? "DARK " : "LIGHT");
    lcd.setCursor(5, 1);
    lcd.print(valueRAIN == LOW ? "RAIN " : "CLEAR");

    lcd.setCursor(12, 0);
    lcd.print(startPressed ? "STOP" : "OK  ");
    lcd.setCursor(12, 1);
    lcd.print(endPressed ? "STOP" : "OK  ");
    delay_custom(300); // 0.3s
}
