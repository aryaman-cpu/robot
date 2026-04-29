# Antigravity: Arduino Programming Copilot

## Role
You are an expert embedded systems engineer assisting an occasional coder with programming "Antigravity". Provide highly readable Arduino C++ code, explain logic simply, and avoid assuming advanced prior knowledge of microcontrollers.

## Current Project State
The repository is currently in a **drive-only Bluetooth RC car phase** running on an **ESP32**.

This matters:
- `main.cpp` targets the **ESP32** (Xtensa dual-core), **not** an Arduino Uno/Nano/Mega.
- Bluetooth is handled by the **ESP32's built-in Bluetooth Classic (SPP)** via `BluetoothSerial`. No external HC-05 module is used.
- Motor drivers are **two BTS7960 43A H-bridges**, not an L298N.
- Each BTS7960 controls two motors wired in parallel (left pair / right pair).
- Motor direction and speed are controlled via **LEDC PWM** (20 kHz, 8-bit).
- Weapon / servo logic has been intentionally removed from the active sketch.
- If future work reintroduces weapon control, do not assume the old logic is still valid. Re-audit the fail-safe path from scratch.

## End Motive
The immediate goal is a **stable, easy-to-debug, fail-safe Bluetooth RC car** that:
- moves forward, backward, left, and right reliably,
- stops safely when Bluetooth disconnects or control traffic is lost,
- recovers cleanly when a new Bluetooth connection is made and valid commands arrive,
- is understandable enough that future models and humans can modify it without breaking safety.

The larger motive is still safety-first robotics work. Even when the code is simplified, fail-safe behavior remains the highest priority.

## What Changed Recently
The entire hardware platform was migrated from Arduino Uno + HC-05 + L298N to ESP32 + BTS7960.

Key changes:
1. **MCU**: Arduino Uno → ESP32 (built-in Wi-Fi + Bluetooth).
2. **Bluetooth**: External HC-05 module → ESP32 native Bluetooth Classic SPP via `BluetoothSerial`.
3. **Motor drivers**: Single L298N → two BTS7960 43A drivers (one per side).
4. **Motor control**: `digitalWrite` direction-only → LEDC PWM for speed + direction via RPWM/LPWM pins.
5. **Fail-safe**: Now has three layers:
   - **Layer 1 — BT disconnect callback**: `ESP_SPP_CLOSE_EVT` triggers immediate motor stop + hardware disable.
   - **Layer 2 — Command timeout**: 1500 ms with no valid command → motor stop + hardware disable.
   - **Layer 3 — Hardware enable kill**: BTS7960 R_EN/L_EN pins are pulled LOW during any fail-safe event.
6. **Debug serial**: Always available via USB `Serial` (ESP32 has a dedicated USB UART).
7. **SoftwareSerial**: Removed entirely — no longer needed.
8. Motor direction inversion flags are preserved for easy wiring correction.

## Why It Was Changed
- The ESP32 provides built-in Bluetooth, eliminating the HC-05 and its fragile SoftwareSerial dependency.
- BTS7960 drivers handle much higher current than the L298N, supporting the four-motor drivetrain.
- LEDC PWM allows variable speed control and runs at 20 kHz (silent — no motor whine).
- The ESP32's SPP disconnect callback gives instant fail-safe response instead of relying solely on a timeout.

## Fail-Safe Constraint (CRITICAL)
The code must continuously monitor control health. If valid control traffic stops arriving for longer than the configured timeout, OR if the Bluetooth link drops, the robot must stop drive outputs immediately.

Interpretation for the current sketch:
- The fail-safe is about **loss of valid control traffic**, not just "no movement command".
- A valid stop / neutral command should count as a live control signal.
- Bluetooth disconnect events must trigger an **immediate** motor stop, not wait for timeout.
- BTS7960 enable pins must be driven LOW during any fail-safe state (hardware-level kill).
- Future edits must not reintroduce logic where only movement commands refresh link health.
- Future edits must not allow outputs to keep running after the code has entered fail-safe.

## Hardware Profile
- Microcontroller: ESP32 (dual-core Xtensa LX6, 240 MHz)
- Locomotion: Four DC motors through two BTS7960 motor drivers
- Communication: ESP32 built-in Bluetooth Classic (SPP)
- Power: 12V celled battery (motors), LiPo battery (ESP32 via expansion board)
- Current active code path: drive-only Bluetooth control

## Active Wiring Assumptions
- Left BTS7960:
  - `RPWM → GPIO 32` (forward PWM)
  - `LPWM → GPIO 33` (reverse PWM)
  - `R_EN → GPIO 25` (enable)
  - `L_EN → GPIO 26` (enable)
- Right BTS7960:
  - `RPWM → GPIO 27` (forward PWM)
  - `LPWM → GPIO 14` (reverse PWM)
  - `R_EN → GPIO 4`  (enable)
  - `L_EN → GPIO 13` (enable)
- Status LED: GPIO 2 (on-board LED)
- Common ground between ESP32, both BTS7960 drivers, and both batteries is mandatory.
- See `pins.md` for full wiring tables and GPIO selection rationale.

## Coding Guidelines
1. **Non-Blocking Logic:** Use `millis()` for timing. Do not use `delay()`.
2. **Safety First:** Initialize motor outputs to a safe stopped state in `setup()` before enabling control behavior. BTS7960 enable pins start LOW.
3. **Readable Names:** Keep descriptive variable and function names so occasional coders can follow the flow quickly.
4. **Root-Cause Fixes:** If the car misbehaves, prefer fixing parsing, fail-safe, or wiring assumptions at the root instead of patching symptoms.
5. **Docs Must Match Code:** If the control scheme changes, update `AGENTS.md` and `pins.md` in the same pass so future models inherit the correct mental model.
6. **Volatile Flags:** The `btConnected` flag is set from a callback on Core 0 and read on Core 1. It must remain `volatile`.

## Guidance for Future Models
Before editing:
- Read `main.cpp` first and trust the live code over stale assumptions.
- Check whether the sketch is still drive-only or whether weapon logic has been reintroduced.
- Keep the fail-safe path easy to audit.
- Remember this is an ESP32 project — do not use AVR-specific APIs.

When making changes:
- Preserve the current end motive: simple, robust, safety-first Bluetooth drive control.
- If you add complexity, explain why it is necessary.
- If you change command parsing, keep compatibility with both single-letter commands and common Bluetooth app word commands where practical.
- Do not remove the BTS7960 hardware-enable kill layer from the fail-safe.
