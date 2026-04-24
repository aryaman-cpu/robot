#include <Arduino.h>
#line 1 "/Users/guranshbir2012/Development/Robotics/Robotics.ino"
#include <SoftwareSerial.h>

// HC-05 Bluetooth Module pins
const int bluetoothRxPin = 2; // Connect to HC-05 TX
const int bluetoothTxPin = 3; // Connect to HC-05 RX
SoftwareSerial bluetooth(bluetoothRxPin, bluetoothTxPin);

// L298N Motor Driver pins
// Since the L298N has 2 output channels, wire the 2 left Johnson motors in
// parallel to OUT1/OUT2, and the 2 right Johnson motors in parallel to
// OUT3/OUT4.
const int leftMotorEnablePin = 5; // PWM pin for left speed
const int leftMotorForwardPin = 7;
const int leftMotorBackwardPin = 8;

const int rightMotorEnablePin = 6; // PWM pin for right speed
const int rightMotorForwardPin = 9;
const int rightMotorBackwardPin = 10;

// ==========================================
// CRITICAL FAIL-SAFE PARAMETERS
// ==========================================
unsigned long lastSignalTime = 0;
// Halt all motors if no Bluetooth command is received for 500 milliseconds.
// Your Bluetooth controller app should ideally send continuous packets while
// driving.
const unsigned long radioSignalTimeout = 500;
bool isFailSafeActive = false;

// Default motor speed (0-255)
int currentSpeed = 255;

#line 33 "/Users/guranshbir2012/Development/Robotics/Robotics.ino"
void setup();
#line 74 "/Users/guranshbir2012/Development/Robotics/Robotics.ino"
void loop();
#line 80 "/Users/guranshbir2012/Development/Robotics/Robotics.ino"
void checkBluetoothCommand();
#line 96 "/Users/guranshbir2012/Development/Robotics/Robotics.ino"
void checkFailSafe();
#line 108 "/Users/guranshbir2012/Development/Robotics/Robotics.ino"
void executeCommand(char command);
#line 149 "/Users/guranshbir2012/Development/Robotics/Robotics.ino"
void moveForward();
#line 159 "/Users/guranshbir2012/Development/Robotics/Robotics.ino"
void moveBackward();
#line 169 "/Users/guranshbir2012/Development/Robotics/Robotics.ino"
void turnLeft();
#line 180 "/Users/guranshbir2012/Development/Robotics/Robotics.ino"
void turnRight();
#line 191 "/Users/guranshbir2012/Development/Robotics/Robotics.ino"
void stopAllMotors();
#line 33 "/Users/guranshbir2012/Development/Robotics/Robotics.ino"
void setup() {
  // Initialize Serial monitor for debugging
  Serial.begin(9600);

  // Initialize HC-05 Bluetooth communication (default baud is usually 9600)
  bluetooth.begin(9600);

  // Configure motor driver pins as outputs
  pinMode(leftMotorEnablePin, OUTPUT);
  pinMode(leftMotorForwardPin, OUTPUT);
  pinMode(leftMotorBackwardPin, OUTPUT);

  pinMode(rightMotorEnablePin, OUTPUT);
  pinMode(rightMotorForwardPin, OUTPUT);
  pinMode(rightMotorBackwardPin, OUTPUT);

  // STARTUP LED TEST:
  // Turn on all logic pins to test Wokwi connections!
  Serial.println("Performing visual LED check...");
  digitalWrite(leftMotorForwardPin, HIGH);
  digitalWrite(leftMotorBackwardPin, HIGH);
  digitalWrite(rightMotorForwardPin, HIGH);
  digitalWrite(rightMotorBackwardPin, HIGH);

  // Need currentSpeed > 0 to enable the pins if analogWrite is used
  analogWrite(leftMotorEnablePin, 255);
  analogWrite(rightMotorEnablePin, 255);

  delay(1000); // Leave them on for 1 full second

  // CRITICAL: Initialize all motor pins to LOW immediately for safety
  // This prevents any accidental movement on startup.
  stopAllMotors();

  // Note: We use millis() and the built-in serial buffer (which uses
  // interrupts) so no hardware interrupts are manually needed for this HC-05
  // setup.
  Serial.println("Antigravity Bluetooth RC System Initialized.");
  Serial.println("Waiting for signal...");
}

void loop() {
  // Non-blocking loop
  checkBluetoothCommand();
  checkFailSafe();
}

void checkBluetoothCommand() {
  if (Serial.available() > 0) {
    char command = Serial.read();

    // Update last signal time to reset the Fail-Safe timer
    lastSignalTime = millis();

    if (isFailSafeActive) {
      Serial.println("Signal restored. Disengaging Fail-Safe.");
      isFailSafeActive = false;
    }

    executeCommand(command);
  }
}

void checkFailSafe() {
  if (!isFailSafeActive) {
    if (millis() - lastSignalTime > radioSignalTimeout) {
      // Signal lost or timed out!
      isFailSafeActive = true;
      Serial.println(
          "CRITICAL: Signal lost! Engaging Fail-Safe. Motors halted.");
      stopAllMotors();
    }
  }
}

void executeCommand(char command) {
  // Simple character-based protocol commonly used by mobile RC apps
  switch (command) {
  case 'F':
  case 'f': // Forward
    moveForward();
    break;
  case 'B':
  case 'b': // Backward
    moveBackward();
    break;
  case 'L':
  case 'l': // Turn Left
    turnLeft();
    break;
  case 'R':
  case 'r': // Turn Right
    turnRight();
    break;
  case 'S':
  case 's': // Stop
    stopAllMotors();
    break;

    // (Optional) You can add logic here if your app sends numbers mapping to
    // speed (0-9) case '0' ... '9':
    //   currentSpeed = map(command - '0', 0, 9, 0, 255);
    //   break;

  default:
    // Unknown character received. Keep driving if running, rely on the
    // Fail-Safe timeout to eventually stop the bot if it's garbage
    // interference.
    break;
  }
}

// ----------------------------------------------------
// MOTOR CONTROL FUNCTIONS
// ----------------------------------------------------

void moveForward() {
  digitalWrite(leftMotorForwardPin, HIGH);
  digitalWrite(leftMotorBackwardPin, LOW);
  analogWrite(leftMotorEnablePin, currentSpeed);

  digitalWrite(rightMotorForwardPin, HIGH);
  digitalWrite(rightMotorBackwardPin, LOW);
  analogWrite(rightMotorEnablePin, currentSpeed);
}

void moveBackward() {
  digitalWrite(leftMotorForwardPin, LOW);
  digitalWrite(leftMotorBackwardPin, HIGH);
  analogWrite(leftMotorEnablePin, currentSpeed);

  digitalWrite(rightMotorForwardPin, LOW);
  digitalWrite(rightMotorBackwardPin, HIGH);
  analogWrite(rightMotorEnablePin, currentSpeed);
}

void turnLeft() {
  // Tank turn: left wheels reverse, right wheels forward
  digitalWrite(leftMotorForwardPin, LOW);
  digitalWrite(leftMotorBackwardPin, HIGH);
  analogWrite(leftMotorEnablePin, currentSpeed);

  digitalWrite(rightMotorForwardPin, HIGH);
  digitalWrite(rightMotorBackwardPin, LOW);
  analogWrite(rightMotorEnablePin, currentSpeed);
}

void turnRight() {
  // Tank turn: left wheels forward, right wheels reverse
  digitalWrite(leftMotorForwardPin, HIGH);
  digitalWrite(leftMotorBackwardPin, LOW);
  analogWrite(leftMotorEnablePin, currentSpeed);

  digitalWrite(rightMotorForwardPin, LOW);
  digitalWrite(rightMotorBackwardPin, HIGH);
  analogWrite(rightMotorEnablePin, currentSpeed);
}

void stopAllMotors() {
  digitalWrite(leftMotorForwardPin, LOW);
  digitalWrite(leftMotorBackwardPin, LOW);
  analogWrite(leftMotorEnablePin, 0);

  digitalWrite(rightMotorForwardPin, LOW);
  digitalWrite(rightMotorBackwardPin, LOW);
  analogWrite(rightMotorEnablePin, 0);
}

