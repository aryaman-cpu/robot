#include <Arduino.h>
#include "BluetoothSerial.h"
#include <string.h>

// ============================================================
// Antigravity — ESP32 Bluetooth RC Car
// ------------------------------------------------------------
// Hardware:
//   - ESP32 with built-in Bluetooth Classic (SPP)
//   - Two BTS7960 motor drivers (left side + right side)
//   - Four DC motors (two per side, wired in parallel)
//   - 12V battery for motors, LiPo for ESP32
//
// Commands accepted from the Bluetooth app:
//   F / FORWARD / GO / 1
//   B / BACK / BACKWARD / REVERSE / 2
//   L / LEFT / 3
//   R / RIGHT / 4
//   S / STOP / BRAKE / IDLE / 0 / 5
//
// Fail-safe (dual-layer):
//   1. Bluetooth disconnect callback → immediate motor stop
//   2. Command timeout (1500 ms)    → motor stop if no valid data
//   Both layers independently disable BTS7960 enable pins.
//
// IMPORTANT HARDWARE NOTE:
//   Each BTS7960 has RPWM / LPWM / R_EN / L_EN pins.
//   RPWM drives forward, LPWM drives reverse.
//   Enable pins are pulled LOW during fail-safe for a
//   hardware-level motor kill on top of the software stop.
// ============================================================

// -----------------------------
// Types & Enums
// -----------------------------
enum class DriveCommand {
    None,
    Forward,
    Backward,
    Left,
    Right,
    Stop
};

// -----------------------------
// Function prototypes
// -----------------------------
void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
void checkBluetoothCommand();
void checkFailSafe();
void executeDriveCommand(DriveCommand command);
DriveCommand readNextCommand();
DriveCommand handleIncomingByte(char incomingByte);
DriveCommand decodeCommandToken(const char *token);
void clearCommandBuffer();
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void stopDriveMotors();
void setDriveOutputs(bool leftForward, bool rightForward);
void enableMotorDrivers();
void disableMotorDrivers();
void initPWM();

// -----------------------------
// Bluetooth
// -----------------------------
// ESP32 has Bluetooth built-in — no external HC-05 module needed.
// Debug output goes to USB Serial (always available on ESP32).
BluetoothSerial SerialBT;
const char *BT_DEVICE_NAME = "Antigravity";

// Modified from the Bluetooth callback context (runs on Core 0).
// Marked volatile so the compiler doesn't cache it on Core 1.
volatile bool btConnected = false;

// -----------------------------
// BTS7960 motor driver pins
// -----------------------------
// Left BTS7960 — controls the two left-side motors (wired in parallel).
const int leftRPWM  = 32;   // Forward PWM
const int leftLPWM  = 33;   // Reverse PWM
const int leftREN   = 25;   // Right-enable
const int leftLEN   = 26;   // Left-enable

// Right BTS7960 — controls the two right-side motors (wired in parallel).
const int rightRPWM = 27;   // Forward PWM
const int rightLPWM = 14;   // Reverse PWM
const int rightREN  = 4;    // Right-enable
const int rightLEN  = 13;   // Left-enable

// On-board LED for visual status (GPIO 2 on most ESP32 dev boards).
const int statusLedPin = 2;

// Set either flag to true if that side runs backwards on your car.
const bool invertLeftMotorDirection  = false;
const bool invertRightMotorDirection = false;

// -----------------------------
// PWM settings (LEDC)
// -----------------------------
// 20 kHz is above the audible range — no motor whine.
// 8-bit resolution gives duty values 0–255.
const int PWM_FREQ       = 20000;
const int PWM_RESOLUTION = 8;
const int PWM_MAX_DUTY   = 255;

// Default drive speed (0–255). Lower for testing, raise for full speed.
const int driveSpeed = 255;

// -----------------------------
// Fail-safe settings
// -----------------------------
unsigned long lastSignalTime         = 0;
const unsigned long signalTimeoutMs  = 1500;
bool isFailSafeActive                = false;

// Command parser buffer
char     incomingToken[20];
uint8_t  incomingTokenLength       = 0;
unsigned long lastBluetoothByteTime = 0;
const unsigned long tokenFlushMs    = 40;

// ============================================================
// Bluetooth SPP event callback
// ============================================================
// Runs in the Bluetooth stack task on Core 0.
// Only touches the volatile bool — no heavy work here.
void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    if (event == ESP_SPP_SRV_OPEN_EVT)
    {
        btConnected = true;
        Serial.println("[BT] Client connected.");
    }
    else if (event == ESP_SPP_CLOSE_EVT)
    {
        btConnected = false;
        Serial.println("[BT] Client disconnected — fail-safe triggered.");
    }
}

// ============================================================
// setup()
// ============================================================
void setup()
{
    Serial.begin(115200);

    // Motor enable pins — set LOW immediately (motors disabled).
    pinMode(leftREN,  OUTPUT);
    pinMode(leftLEN,  OUTPUT);
    pinMode(rightREN, OUTPUT);
    pinMode(rightLEN, OUTPUT);
    disableMotorDrivers();

    // Status LED
    pinMode(statusLedPin, OUTPUT);
    digitalWrite(statusLedPin, LOW);

    // Initialise LEDC PWM on the four motor-PWM pins.
    initPWM();

    // Safety first: all PWM duty to zero.
    stopDriveMotors();

    // Start Bluetooth with event callback.
    SerialBT.register_callback(btCallback);
    SerialBT.begin(BT_DEVICE_NAME);

    Serial.println("Antigravity ESP32 RC car ready.");
    Serial.print("Bluetooth name: ");
    Serial.println(BT_DEVICE_NAME);
    Serial.println("Commands: F/B/L/R/S or FORWARD/LEFT/STOP etc.");

    lastSignalTime = millis();
}

// ============================================================
// loop()
// ============================================================
void loop()
{
    // If Bluetooth dropped, force-stop immediately.
    if (!btConnected && !isFailSafeActive)
    {
        isFailSafeActive = true;
        stopDriveMotors();
        disableMotorDrivers();
    }

    checkBluetoothCommand();
    checkFailSafe();
}

// ============================================================
// Bluetooth command handling
// ============================================================
void checkBluetoothCommand()
{
    if (!btConnected)
    {
        // Poka-Yoke: Flush any stale data to prevent processing after disconnect
        while (SerialBT.available() > 0)
        {
            SerialBT.read();
        }
        clearCommandBuffer();
        return;
    }

    DriveCommand command = readNextCommand();

    if (command == DriveCommand::None)
    {
        return;
    }

    // A valid command arrived — refresh the fail-safe timer.
    lastSignalTime = millis();

    if (isFailSafeActive)
    {
        isFailSafeActive = false;
        enableMotorDrivers();
        Serial.println("Signal restored. Control resumed.");
    }

    executeDriveCommand(command);
}

// ============================================================
// Fail-safe (timeout path)
// ============================================================
void checkFailSafe()
{
    if (isFailSafeActive)
    {
        return;
    }

    if (millis() - lastSignalTime >= signalTimeoutMs)
    {
        isFailSafeActive = true;
        stopDriveMotors();
        disableMotorDrivers();
        Serial.println("Fail-safe active: no command received.");
    }
}

// ============================================================
// Drive command dispatcher
// ============================================================
void executeDriveCommand(DriveCommand command)
{
    switch (command)
    {
    case DriveCommand::Forward:
        moveForward();
        break;
    case DriveCommand::Backward:
        moveBackward();
        break;
    case DriveCommand::Left:
        turnLeft();
        break;
    case DriveCommand::Right:
        turnRight();
        break;
    case DriveCommand::Stop:
    default:
        stopDriveMotors();
        break;
    }
}

// ============================================================
// Read next complete command from Bluetooth serial
// ============================================================
DriveCommand readNextCommand()
{
    while (SerialBT.available() > 0)
    {
        DriveCommand cmd = handleIncomingByte(static_cast<char>(SerialBT.read()));
        if (cmd != DriveCommand::None)
        {
            return cmd;
        }
    }

    // Flush a partial token if no new bytes arrived within tokenFlushMs.
    if (incomingTokenLength > 0 &&
        millis() - lastBluetoothByteTime >= tokenFlushMs)
    {
        incomingToken[incomingTokenLength] = '\0';
        DriveCommand cmd = decodeCommandToken(incomingToken);
        clearCommandBuffer();
        return cmd;
    }

    return DriveCommand::None;
}

// ============================================================
// Byte-level parser (single-char lookahead + word tokens)
// ============================================================
DriveCommand handleIncomingByte(char incomingByte)
{
    lastBluetoothByteTime = millis();

    // Normalise to lowercase.
    if (incomingByte >= 'A' && incomingByte <= 'Z')
    {
        incomingByte = incomingByte - 'A' + 'a';
    }

    const bool isLetter = incomingByte >= 'a' && incomingByte <= 'z';
    const bool isDigit  = incomingByte >= '0' && incomingByte <= '9';
    const bool couldBeSingleCommand =
        incomingByte == 'f' || incomingByte == 'b' ||
        incomingByte == 'l' || incomingByte == 'r' ||
        incomingByte == 's' || incomingByte == '0' ||
        incomingByte == '1' || incomingByte == '2' ||
        incomingByte == '3' || incomingByte == '4' ||
        incomingByte == '5';

    if (isLetter || isDigit)
    {
        // Single-char lookahead: if the buffer holds exactly one valid
        // drive letter and the new byte could also be a standalone
        // command, dispatch the buffered char immediately.
        if (incomingTokenLength == 1 && couldBeSingleCommand)
        {
            DriveCommand bufferedCmd = decodeCommandToken(incomingToken);
            if (bufferedCmd != DriveCommand::None)
            {
                incomingToken[0] = incomingByte;
                incomingToken[1] = '\0';
                incomingTokenLength = 1;
                return bufferedCmd;
            }
        }

        if (incomingTokenLength < sizeof(incomingToken) - 1)
        {
            incomingToken[incomingTokenLength++] = incomingByte;
        }
        else
        {
            // Buffer overflow — clear and keep the current byte.
            clearCommandBuffer();
            incomingToken[incomingTokenLength++] = incomingByte;
        }
        return DriveCommand::None;
    }

    // Non-alphanumeric byte acts as a delimiter.
    if (incomingTokenLength == 0)
    {
        return DriveCommand::None;
    }

    incomingToken[incomingTokenLength] = '\0';
    DriveCommand cmd = decodeCommandToken(incomingToken);
    clearCommandBuffer();
    return cmd;
}

// ============================================================
// Token-to-command decoder
// ============================================================
DriveCommand decodeCommandToken(const char *token)
{
    if (strcmp(token, "f") == 0 || strcmp(token, "forward") == 0 ||
        strcmp(token, "go") == 0 || strcmp(token, "ahead") == 0 ||
        strcmp(token, "1") == 0)
    {
        return DriveCommand::Forward;
    }

    if (strcmp(token, "b") == 0 || strcmp(token, "back") == 0 ||
        strcmp(token, "backward") == 0 || strcmp(token, "reverse") == 0 ||
        strcmp(token, "2") == 0)
    {
        return DriveCommand::Backward;
    }

    if (strcmp(token, "l") == 0 || strcmp(token, "left") == 0 ||
        strcmp(token, "3") == 0)
    {
        return DriveCommand::Left;
    }

    if (strcmp(token, "r") == 0 || strcmp(token, "right") == 0 ||
        strcmp(token, "4") == 0)
    {
        return DriveCommand::Right;
    }

    if (strcmp(token, "s") == 0 || strcmp(token, "stop") == 0 ||
        strcmp(token, "brake") == 0 || strcmp(token, "idle") == 0 ||
        strcmp(token, "0") == 0 || strcmp(token, "5") == 0)
    {
        return DriveCommand::Stop;
    }

    return DriveCommand::None;
}

void clearCommandBuffer()
{
    incomingTokenLength = 0;
    incomingToken[0] = '\0';
}

// ============================================================
// Motor control — BTS7960 via LEDC PWM
// ============================================================
// Each BTS7960 has two PWM inputs:
//   RPWM — drive forward (HIGH side)
//   LPWM — drive reverse  (LOW side)
// To go forward:  RPWM = duty, LPWM = 0
// To go reverse:  RPWM = 0,    LPWM = duty
// To coast/stop:  RPWM = 0,    LPWM = 0

void moveForward()
{
    digitalWrite(statusLedPin, HIGH);
    setDriveOutputs(true, true);
}

void moveBackward()
{
    digitalWrite(statusLedPin, HIGH);
    setDriveOutputs(false, false);
}

void turnLeft()
{
    digitalWrite(statusLedPin, HIGH);
    setDriveOutputs(false, true);
}

void turnRight()
{
    digitalWrite(statusLedPin, HIGH);
    setDriveOutputs(true, false);
}

void setDriveOutputs(bool leftForward, bool rightForward)
{
    if (invertLeftMotorDirection)
    {
        leftForward = !leftForward;
    }
    if (invertRightMotorDirection)
    {
        rightForward = !rightForward;
    }

    // Left BTS7960
    if (leftForward)
    {
        ledcWrite(leftRPWM, driveSpeed);
        ledcWrite(leftLPWM, 0);
    }
    else
    {
        ledcWrite(leftRPWM, 0);
        ledcWrite(leftLPWM, driveSpeed);
    }

    // Right BTS7960
    if (rightForward)
    {
        ledcWrite(rightRPWM, driveSpeed);
        ledcWrite(rightLPWM, 0);
    }
    else
    {
        ledcWrite(rightRPWM, 0);
        ledcWrite(rightLPWM, driveSpeed);
    }
}

void stopDriveMotors()
{
    digitalWrite(statusLedPin, LOW);

    // Zero all PWM outputs — motors coast to a stop.
    ledcWrite(leftRPWM,  0);
    ledcWrite(leftLPWM,  0);
    ledcWrite(rightRPWM, 0);
    ledcWrite(rightLPWM, 0);
}

// ============================================================
// BTS7960 enable / disable
// ============================================================
// Pulling all four enable lines LOW is a hardware-level motor kill.
// This is the second layer of fail-safe on top of zeroing PWM.

void enableMotorDrivers()
{
    digitalWrite(leftREN,  HIGH);
    digitalWrite(leftLEN,  HIGH);
    digitalWrite(rightREN, HIGH);
    digitalWrite(rightLEN, HIGH);
}

void disableMotorDrivers()
{
    digitalWrite(leftREN,  LOW);
    digitalWrite(leftLEN,  LOW);
    digitalWrite(rightREN, LOW);
    digitalWrite(rightLEN, LOW);
}

// ============================================================
// LEDC PWM initialisation
// ============================================================
void initPWM()
{
    // Attach each BTS7960 PWM pin to the LEDC controller.
    // ledcAttach(pin, frequency, resolution) — ESP32 Arduino Core 3.x API.
    ledcAttach(leftRPWM,  PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(leftLPWM,  PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(rightRPWM, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(rightLPWM, PWM_FREQ, PWM_RESOLUTION);
}
