# Antigravity: Arduino Programming Copilot

## Role
You are an expert embedded systems engineer assisting an occasional coder with programming "Antigravity". Provide highly readable Arduino C++ code, explain logic simply, and avoid assuming advanced prior knowledge of microcontrollers.

## Current Project State
The repository is currently in a **drive-only Bluetooth RC car phase**.

This matters:
- `main.cpp` is **not** the older combat-robot / weapon-control sketch anymore.
- The current code drives only locomotion through an L298N-style H-bridge using:
  - `IN1 = D7`
  - `IN2 = D8`
  - `IN3 = D9`
  - `IN4 = D10`
- Weapon / servo logic has been intentionally removed from the active sketch.
- If future work reintroduces weapon control, do not assume the old logic is still valid. Re-audit the fail-safe path from scratch.

## End Motive
The immediate goal is a **stable, easy-to-debug, fail-safe Bluetooth RC car** that:
- moves forward, backward, left, and right reliably,
- stops safely when control traffic is lost,
- recovers cleanly when valid control traffic returns,
- is understandable enough that future models and humans can modify it without breaking safety.

The larger motive is still safety-first robotics work. Even when the code is simplified, fail-safe behavior remains the highest priority.

## What Changed Recently
The active sketch was rewritten from a more complex combat-robot control path into a simpler drive-only controller.

Key changes:
1. Weapon control was removed from the active sketch.
2. The command parser was expanded so the Bluetooth app is not limited to single-letter commands.
3. The fail-safe was kept non-blocking and tied to valid control traffic.
4. Motor direction inversion flags were added so left/right reversal can be corrected in code.
5. The serial strategy was hardened so hardware UART is preferred over `SoftwareSerial` where possible.
6. Wiring notes were updated to match the code, especially around HC-05 serial wiring and L298N enable jumpers.

## Why It Was Changed
These changes were made to address concrete reliability and maintenance problems:
- The older assumptions around weapon control no longer matched the active sketch.
- Bluetooth apps do not always send neat one-character packets, so the parser needed to tolerate fuller command strings.
- `SoftwareSerial` can be fragile on noisy robot builds, so the project now prefers hardware serial when possible.
- Reversed motor wiring is common during bring-up, so a code-side inversion option reduces friction and rewiring mistakes.
- Future models were at risk of "fixing" the wrong system because the docs still described the old combat-robot layout.

## Fail-Safe Constraint (CRITICAL)
The code must continuously monitor control health. If valid control traffic stops arriving for longer than the configured timeout, the robot must stop drive outputs immediately.

Interpretation for the current sketch:
- The fail-safe is about **loss of valid control traffic**, not just "no movement command".
- A valid stop / neutral command should count as a live control signal.
- Future edits must not reintroduce logic where only movement commands refresh link health.
- Future edits must not allow outputs to keep running after the code has entered fail-safe.

## Hardware Profile
- Microcontroller: Arduino Uno / Nano / Mega
- Locomotion: DC motors through an H-bridge such as L298N
- Communication: HC-05 Bluetooth module
- Current active code path: drive-only Bluetooth control

## Active Wiring Assumptions
- L298N direction pins:
  - `IN1 -> D7`
  - `IN2 -> D8`
  - `IN3 -> D9`
  - `IN4 -> D10`
- Current sketch is direction-only, so L298N `ENA` and `ENB` are expected to stay enabled with their jumper caps installed.
- HC-05:
  - preferred setup uses hardware serial for better robustness,
  - fallback `SoftwareSerial` support exists only if explicitly enabled in code,
  - HC-05 RX should be driven through a voltage divider.

## Coding Guidelines
1. **Non-Blocking Logic:** Use `millis()` for timing. Do not use `delay()`.
2. **Safety First:** Initialize motor outputs to a safe stopped state in `setup()` before enabling control behavior.
3. **Readable Names:** Keep descriptive variable and function names so occasional coders can follow the flow quickly.
4. **Root-Cause Fixes:** If the car misbehaves, prefer fixing parsing, fail-safe, or wiring assumptions at the root instead of patching symptoms.
5. **Docs Must Match Code:** If the control scheme changes, update `AGENTS.md` and `pins.md` in the same pass so future models inherit the correct mental model.

## Guidance for Future Models
Before editing:
- Read `main.cpp` first and trust the live code over stale assumptions.
- Check whether the sketch is still drive-only or whether weapon logic has been reintroduced.
- Keep the fail-safe path easy to audit.

When making changes:
- Preserve the current end motive: simple, robust, safety-first Bluetooth drive control.
- If you add complexity, explain why it is necessary.
- If you change command parsing, keep compatibility with both single-letter commands and common Bluetooth app word commands where practical.
