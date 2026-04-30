// ================= Bluetooth =================
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;
 
// ================= LEFT SIDE =================
#define RPWM_L 25
#define LPWM_L 26
#define REN_L  27
#define LEN_L  14
 
// ================= RIGHT SIDE =================
#define RPWM_R 32
#define LPWM_R 33
#define REN_R  12
#define LEN_R  13
 
// ================= PWM =================
const int freq = 15000;
const int resolution = 8;
 
// ================= SPEED =================
int baseSpeed = 200;
float turnFactor = 0.5;
 
// ================= FAILSAFE =================
unsigned long lastCommandTime = 0;
const int timeout = 500;
 
// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  SerialBT.begin("OFFROAD_CAR");
 
  pinMode(REN_L, OUTPUT);
  pinMode(LEN_L, OUTPUT);
  pinMode(REN_R, OUTPUT);
  pinMode(LEN_R, OUTPUT);
 
  digitalWrite(REN_L, HIGH);
  digitalWrite(LEN_L, HIGH);
  digitalWrite(REN_R, HIGH);
  digitalWrite(LEN_R, HIGH);
 
  // ESP32 3.x PWM
  ledcAttach(RPWM_L, freq, resolution);
  ledcAttach(LPWM_L, freq, resolution);
  ledcAttach(RPWM_R, freq, resolution);
  ledcAttach(LPWM_R, freq, resolution);
}
 
// ================= MOTOR CONTROL =================
void setMotorLeft(int speed) {
  speed = constrain(speed, -255, 255);
 
  if (speed > 0) {
    ledcWrite(RPWM_L, speed);
    ledcWrite(LPWM_L, 0);
  } else if (speed < 0) {
    ledcWrite(RPWM_L, 0);
    ledcWrite(LPWM_L, -speed);
  } else {
    ledcWrite(RPWM_L, 0);
    ledcWrite(LPWM_L, 0);
  }
}
 
void setMotorRight(int speed) {
  speed = constrain(speed, -255, 255);
 
  if (speed > 0) {
    ledcWrite(RPWM_R, speed);
    ledcWrite(LPWM_R, 0);
  } else if (speed < 0) {
    ledcWrite(RPWM_R, 0);
    ledcWrite(LPWM_R, -speed);
  } else {
    ledcWrite(RPWM_R, 0);
    ledcWrite(LPWM_R, 0);
  }
}
 
void move(int leftSpeed, int rightSpeed) {
  setMotorLeft(leftSpeed);
  setMotorRight(rightSpeed);
}
 
// ================= LOOP =================
void loop() {
 
  if (SerialBT.available()) {
    char cmd = SerialBT.read();
    lastCommandTime = millis();
 
    switch (cmd) {
 
      // ===== STOP (instant) =====
      case 'S': 
        move(0, 0); 
        break;
 
      // ===== Forward =====
      case 'F': move(baseSpeed, baseSpeed); break;
 
      // ===== Reverse =====
      case 'B': move(-baseSpeed, -baseSpeed); break;
 
      // ===== Forward Turns =====
      case 'G': move(baseSpeed * turnFactor, baseSpeed); break;
      case 'I': move(baseSpeed, baseSpeed * turnFactor); break;
 
      // ===== Reverse Turns =====
      case 'J': move(-baseSpeed * turnFactor, -baseSpeed); break;
      case 'H': move(-baseSpeed, -baseSpeed * turnFactor); break;
 
      // ===== Spin =====
      case 'L': move(-baseSpeed, baseSpeed); break;
      case 'R': move(baseSpeed, -baseSpeed); break;
 
      // ===== Speed Control =====
      case '0': baseSpeed = 0; break;
      case '1': baseSpeed = 80; break;
      case '2': baseSpeed = 120; break;
      case '3': baseSpeed = 160; break;
      case '4': baseSpeed = 200; break;
      case '5': baseSpeed = 255; break;
    }
  }
 
  // ===== FAILSAFE (backup only) =====
  if (millis() - lastCommandTime > timeout) {
    move(0, 0);
  }
           }
