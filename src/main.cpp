#include "delay_driver.h"
#include "gpio_driver.h"
#include "serial_driver.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

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
const int MOTOR_SPEED = 25; // PWM Speed (0-255)
// ===== Motor State=====
enum MotorState {
  MOTOR_STOPPED,
  MOTOR_FORWARD, // PHƠI đồ
  MOTOR_REVERSE  // THU đồ
};

MotorState currentMotorState = MOTOR_STOPPED;

// ===== LATCH flags =====
bool startLimitLatched = false;
bool endLimitLatched = false;

// ===== MOTOR CONTROL =====
void motorStop() {
  digitalWrite_custom(MOTOR_IN1, LOW);
  digitalWrite_custom(MOTOR_IN2, LOW);
}

void motorForward() { // PHƠI đồ
  analogWrite(MOTOR_IN1, MOTOR_SPEED);
  digitalWrite_custom(MOTOR_IN2, LOW);
}

void motorReverse() { // THU đồ
  digitalWrite_custom(MOTOR_IN1, LOW);
  analogWrite(MOTOR_IN2, MOTOR_SPEED);
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

  // ===== LCD Shown =====
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LDR: ");
  lcd.setCursor(0, 1);
  lcd.print("RAIN:");

  Serial.println("=== SETUP COMPLETE ===\n");
}

void loop() {
  // ===== Read Sensor =====
  int valueLDR = digitalRead_custom(LDR_DO);
  int valueRAIN = digitalRead_custom(RAIN_DO);
  // ===== Read Limit Switches =====
  // Lưu ý: Tùy vào cách đấu dây (NC hay NO), nếu chưa bấm là HIGH, bấm là LOW
  // thì cần thêm dấu ! Ở đây giả sử: Bấm vào là HIGH (1), thả ra là LOW (0) dựa
  // theo logic cũ của bạn.
  int startPressed = digitalRead(LIMIT_SWITCH_START);
  int endPressed = digitalRead(LIMIT_SWITCH_END);

  // ===== Definite Motor Direction Logic =====
  // Logic: Trời sáng (LOW?) VÀ Không mưa (HIGH?) -> Phơi đồ (FORWARD)
  // Ngược lại -> Thu đồ (REVERSE)
  MotorState desiredMotorState =
      (valueLDR == LOW && valueRAIN == HIGH) ? MOTOR_FORWARD : MOTOR_REVERSE;

  // ===== State Machine =====
  switch (currentMotorState) {

  // --- 1: Motor Stopped ---
  case MOTOR_STOPPED:
    // Logic mới: Tách riêng điều kiện cho từng chiều để tránh bị kẹt

    if (desiredMotorState == MOTOR_FORWARD) {
      // Muốn đi TIẾN: Chỉ cần chưa chạm công tắc giới hạn ĐÍCH (End)
      // Và reset cờ latch của chiều ngược lại nếu cần
      if (!endPressed) {
        endLimitLatched = false; // Reset latch đích
        motorForward();
        currentMotorState = MOTOR_FORWARD;
      }
    } else if (desiredMotorState == MOTOR_REVERSE) {
      // Muốn đi LÙI: Chỉ cần chưa chạm công tắc giới hạn ĐẦU (Start)
      if (!startPressed) {
        startLimitLatched = false; // Reset latch đầu
        motorReverse();
        currentMotorState = MOTOR_REVERSE;
      }
    }
    break;

  // --- 2: Motor Forward (Phơi) ---
  case MOTOR_FORWARD:
    // Ưu tiên 1: Chạm công tắc hành trình thì dừng
    if (endPressed) {
      motorStop();
      currentMotorState = MOTOR_STOPPED;
      endLimitLatched = true;
    }
    // Ưu tiên 2: Cảm biến thay đổi ý định (trời mưa/tối)
    else if (desiredMotorState == MOTOR_REVERSE) {
      motorStop();
      delay_custom(500); // Đợi dừng hẳn
      motorReverse();
      currentMotorState = MOTOR_REVERSE;
    }
    break;

  // --- 3: Motor Reverse (Thu) ---
  case MOTOR_REVERSE:
    // Ưu tiên 1: Chạm công tắc hành trình thì dừng
    if (startPressed) {
      motorStop();
      currentMotorState = MOTOR_STOPPED;
      startLimitLatched = true;
    }
    // Ưu tiên 2: Cảm biến thay đổi ý định (trời nắng lại)
    else if (desiredMotorState == MOTOR_FORWARD) {
      motorStop();
      delay_custom(500); // Đợi dừng hẳn
      motorForward();
      currentMotorState = MOTOR_FORWARD;
    }
    break; // <--- QUAN TRỌNG: Đã thêm break để không trôi xuống default

  default:
    motorStop();
    currentMotorState = MOTOR_STOPPED;
    break;
  }

  // ===== LCD Modify =====
  // Phần hiển thị giữ nguyên
  lcd.setCursor(5, 0);
  lcd.print(valueLDR == HIGH
                ? "DARK "
                : "LIGHT"); // Kiểm tra lại logic LDR thực tế của bạn
  lcd.setCursor(5, 1);
  lcd.print(valueRAIN == LOW ? "RAIN " : "CLEAR");

  lcd.setCursor(12, 0);
  lcd.print(startPressed ? "STOP" : "OK  ");
  lcd.setCursor(12, 1);
  lcd.print(endPressed ? "STOP" : "OK  ");

  delay_custom(100); // Giảm delay một chút để phản hồi nhanh hơn
}
