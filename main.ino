#include <Arduino.h>
#include <string.h>

// ============================================================
// Antigravity / RC Car Bluetooth Drive Code
// ------------------------------------------------------------
// Control commands accepted from the Bluetooth app:
//   F / FORWARD / GO
//   B / BACK / BACKWARD / REVERSE
//   L / LEFT
//   R / RIGHT
//   S / STOP / BRAKE / IDLE
//
// This version is intentionally simple:
// - Only drive control is included.
// - No delay() calls are used.
// - If valid commands stop arriving, the fail-safe stops the car.
// - The next valid command automatically restores control.
//
// IMPORTANT HARDWARE NOTE:
// This code only drives IN1/IN2/IN3/IN4.
// For an L298N, ENA and ENB must stay enabled with their jumper caps fitted.
// ============================================================

// -----------------------------
// Function prototypes
// -----------------------------
void checkBluetoothCommand();
void checkFailSafe();
void executeDriveCommand(char command);
bool isValidDriveCommand(char command);
bool readNextCommand(char &command);
bool handleIncomingByte(char incomingByte, char &command);
char decodeCommandToken(const char *token);
void clearCommandBuffer();
void debugPrint(const __FlashStringHelper *message);
void debugPrintln(const __FlashStringHelper *message);
void debugPrintln(const char *message);
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void stopDriveMotors();
void setDriveOutputs(bool leftForward, bool rightForward);

// -----------------------------
// HC-05 Bluetooth wiring
// -----------------------------
// Preferred: use a hardware UART for the HC-05.
// - Uno / Nano: HC-05 TX -> D0, HC-05 RX -> D1 through a voltage divider
// - Mega: HC-05 TX -> RX1 (19), HC-05 RX -> TX1 (18) through a divider
//
// If you absolutely must keep HC-05 on D2/D3, set this to 1 and rewire to:
//   HC-05 TX -> D2, HC-05 RX -> D3 through a divider
#define USE_SOFTWARE_SERIAL_FOR_HC05 0

#if USE_SOFTWARE_SERIAL_FOR_HC05
#include <SoftwareSerial.h>
const int bluetoothRxPin = 2;
const int bluetoothTxPin = 3;
SoftwareSerial controlSerial(bluetoothRxPin, bluetoothTxPin);
const bool debugSerialEnabled = true;
#define DEBUG_SERIAL Serial
#elif defined(UBRR1H)
#define CONTROL_SERIAL Serial1
#define DEBUG_SERIAL Serial
const bool debugSerialEnabled = true;
#else
#define CONTROL_SERIAL Serial
#define DEBUG_SERIAL Serial
const bool debugSerialEnabled = false;
#endif

// -----------------------------
// Motor driver direction pins
// -----------------------------
const int motorIn1Pin = 7;
const int motorIn2Pin = 8;
const int motorIn3Pin = 9;
const int motorIn4Pin = 10;

const int statusLedPin = LED_BUILTIN;

// Set either flag to true if that side runs backwards on your car.
const bool invertLeftMotorDirection = false;
const bool invertRightMotorDirection = false;

// -----------------------------
// Fail-safe settings
// -----------------------------
unsigned long lastSignalTime = 0;
const unsigned long radioSignalTimeoutMs = 1500;
bool isFailSafeActive = false;

char incomingToken[20];
uint8_t incomingTokenLength = 0;
unsigned long lastBluetoothByteTime = 0;
const unsigned long tokenFlushTimeoutMs = 40;

void setup() {
#if USE_SOFTWARE_SERIAL_FOR_HC05
  DEBUG_SERIAL.begin(115200);
  controlSerial.begin(9600);
  controlSerial.listen();
#elif defined(UBRR1H)
  DEBUG_SERIAL.begin(115200);
  CONTROL_SERIAL.begin(9600);
#else
  CONTROL_SERIAL.begin(9600);
#endif

  pinMode(motorIn1Pin, OUTPUT);
  pinMode(motorIn2Pin, OUTPUT);
  pinMode(motorIn3Pin, OUTPUT);
  pinMode(motorIn4Pin, OUTPUT);
  pinMode(statusLedPin, OUTPUT);

  // Safety first: stop everything before doing anything else.
  stopDriveMotors();
  digitalWrite(statusLedPin, LOW);

  debugPrintln(F("Bluetooth RC car ready."));
  debugPrintln(F("Commands: F/B/L/R/S or words like FORWARD/LEFT/STOP"));

  lastSignalTime = millis();
}

void loop() {
  checkBluetoothCommand();
  checkFailSafe();
}

void checkBluetoothCommand() {
  char command = '\0';

  if (!readNextCommand(command)) {
    return;
  }

  lastSignalTime = millis();

  if (isFailSafeActive) {
    isFailSafeActive = false;
    debugPrintln(F("Signal restored. Control resumed."));
  }

  executeDriveCommand(command);
}

void checkFailSafe() {
  if (isFailSafeActive) {
    return;
  }

  if (millis() - lastSignalTime >= radioSignalTimeoutMs) {
    isFailSafeActive = true;
    stopDriveMotors();
    debugPrintln(F("Fail-safe active: no command received."));
  }
}

void executeDriveCommand(char command) {
  switch (command) {
  case 'f':
    moveForward();
    break;
  case 'b':
    moveBackward();
    break;
  case 'l':
    turnLeft();
    break;
  case 'r':
    turnRight();
    break;
  case 's':
  default:
    stopDriveMotors();
    break;
  }
}

bool isValidDriveCommand(char command) {
  switch (command) {
  case 'f':
  case 'b':
  case 'l':
  case 'r':
  case 's':
    return true;
  default:
    return false;
  }
}

bool readNextCommand(char &command) {
  command = '\0';

#if USE_SOFTWARE_SERIAL_FOR_HC05
  while (controlSerial.available() > 0) {
    if (handleIncomingByte(static_cast<char>(controlSerial.read()), command)) {
      return true;
    }
  }
#else
  while (CONTROL_SERIAL.available() > 0) {
    if (handleIncomingByte(static_cast<char>(CONTROL_SERIAL.read()), command)) {
      return true;
    }
  }
#endif

  if (incomingTokenLength > 0 &&
      millis() - lastBluetoothByteTime >= tokenFlushTimeoutMs) {
    incomingToken[incomingTokenLength] = '\0';
    command = decodeCommandToken(incomingToken);
    clearCommandBuffer();
    return isValidDriveCommand(command);
  }

  return false;
}

bool handleIncomingByte(char incomingByte, char &command) {
  lastBluetoothByteTime = millis();

  if (incomingByte >= 'A' && incomingByte <= 'Z') {
    incomingByte = incomingByte - 'A' + 'a';
  }

  const bool isLetter = incomingByte >= 'a' && incomingByte <= 'z';
  const bool isDigit = incomingByte >= '0' && incomingByte <= '9';
  const bool couldBeSingleCommand =
      incomingByte == 'f' || incomingByte == 'b' || incomingByte == 'l' ||
      incomingByte == 'r' || incomingByte == 's' || incomingByte == '0' ||
      incomingByte == '1' || incomingByte == '2' || incomingByte == '3' ||
      incomingByte == '4' || incomingByte == '5';

  if (isLetter || isDigit) {
    if (incomingTokenLength == 1 && isValidDriveCommand(incomingToken[0]) &&
        couldBeSingleCommand) {
      command = incomingToken[0];
      incomingToken[0] = incomingByte;
      incomingToken[1] = '\0';
      incomingTokenLength = 1;
      return true;
    }

    if (incomingTokenLength < sizeof(incomingToken) - 1) {
      incomingToken[incomingTokenLength++] = incomingByte;
    } else {
      // BUG FIX: After clearing an overflowed buffer, keep the current byte
      // so the first character of the next token is not silently dropped.
      clearCommandBuffer();
      incomingToken[incomingTokenLength++] = incomingByte;
    }
    return false;
  }

  if (incomingTokenLength == 0) {
    return false;
  }

  incomingToken[incomingTokenLength] = '\0';
  command = decodeCommandToken(incomingToken);
  clearCommandBuffer();
  return isValidDriveCommand(command);
}

char decodeCommandToken(const char *token) {
  if (strcmp(token, "f") == 0 || strcmp(token, "forward") == 0 ||
      strcmp(token, "go") == 0 || strcmp(token, "ahead") == 0 ||
      strcmp(token, "1") == 0) {
    return 'f';
  }

  if (strcmp(token, "b") == 0 || strcmp(token, "back") == 0 ||
      strcmp(token, "backward") == 0 || strcmp(token, "reverse") == 0 ||
      strcmp(token, "2") == 0) {
    return 'b';
  }

  if (strcmp(token, "l") == 0 || strcmp(token, "left") == 0 ||
      strcmp(token, "3") == 0) {
    return 'l';
  }

  if (strcmp(token, "r") == 0 || strcmp(token, "right") == 0 ||
      strcmp(token, "4") == 0) {
    return 'r';
  }

  if (strcmp(token, "s") == 0 || strcmp(token, "stop") == 0 ||
      strcmp(token, "brake") == 0 || strcmp(token, "idle") == 0 ||
      strcmp(token, "0") == 0 || strcmp(token, "5") == 0) {
    return 's';
  }

  return '\0';
}

void clearCommandBuffer() {
  incomingTokenLength = 0;
  incomingToken[0] = '\0';
}

void debugPrint(const __FlashStringHelper *message) {
  if (debugSerialEnabled) {
    DEBUG_SERIAL.print(message);
  }
}

void debugPrintln(const __FlashStringHelper *message) {
  if (debugSerialEnabled) {
    DEBUG_SERIAL.println(message);
  }
}

void debugPrintln(const char *message) {
  if (debugSerialEnabled) {
    DEBUG_SERIAL.println(message);
  }
}

void moveForward() {
  digitalWrite(statusLedPin, HIGH);
  setDriveOutputs(true, true);
}

void moveBackward() {
  digitalWrite(statusLedPin, HIGH);
  setDriveOutputs(false, false);
}

void turnLeft() {
  digitalWrite(statusLedPin, HIGH);
  setDriveOutputs(false, true);
}

void turnRight() {
  digitalWrite(statusLedPin, HIGH);
  setDriveOutputs(true, false);
}

void setDriveOutputs(bool leftForward, bool rightForward) {
  if (invertLeftMotorDirection) {
    leftForward = !leftForward;
  }

  if (invertRightMotorDirection) {
    rightForward = !rightForward;
  }

  digitalWrite(motorIn1Pin, leftForward ? HIGH : LOW);
  digitalWrite(motorIn2Pin, leftForward ? LOW : HIGH);
  digitalWrite(motorIn3Pin, rightForward ? HIGH : LOW);
  digitalWrite(motorIn4Pin, rightForward ? LOW : HIGH);
}

void stopDriveMotors() {
  digitalWrite(statusLedPin, LOW);

  // Coast to a stop by removing drive signals.
  // This is gentler on the power rail than active braking.
  digitalWrite(motorIn1Pin, LOW);
  digitalWrite(motorIn2Pin, LOW);
  digitalWrite(motorIn3Pin, LOW);
  digitalWrite(motorIn4Pin, LOW);
}
