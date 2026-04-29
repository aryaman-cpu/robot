
### 📡 1. HC-05 Bluetooth Module
| HC-05 Pin | Arduino Pin | Notes |
| :--- | :--- | :--- |
| **TX** (Transmit) | **D0 / RX** on Uno/Nano, **RX1 / D19** on Mega | Preferred hardware serial receive pin |
| **RX** (Receive) | **D1 / TX** on Uno/Nano, **TX1 / D18** on Mega | Use a resistor divider here because HC-05 RX expects 3.3V logic |
| **VCC** | **5V** | Power |
| **GND** | **GND** | Ground |

**Optional fallback:**  
If you change `USE_SOFTWARE_SERIAL_FOR_HC05` to `1` in `main.cpp`, then wire:
- HC-05 **TX** -> **D2**
- HC-05 **RX** -> **D3** through a voltage divider

---

### 🏎️ 2. L298N Motor Driver
*Note: The current sketch is direction-only, so keep the physical jumper caps installed on **ENA** and **ENB**. That holds both motor channels enabled all the time.*

| L298N Pin | Arduino Pin | Function |
| :--- | :--- | :--- |
| **ENA** (Enable A) | **Leave jumper installed** | Left motor always enabled |
| **IN1** | **D7** | Left Motor Forward |
| **IN2** | **D8** | Left Motor Backward |
| **IN3** | **D9** | Right Motor Forward |
| **IN4** | **D10** | Right Motor Backward |
| **ENB** (Enable B) | **Leave jumper installed** | Right motor always enabled |

**L298N Power Connections:**
*   **12V Pin:** Connect to your main battery Positive (+).
*   **GND Pin:** Connect to your main battery Negative (-) **AND** connect this to the Arduino's **GND** pin. *(Sharing a common ground between the Arduino and the motor driver is mandatory, or the signals won't work).*
*   **5V Pin:** Leave empty (unless you are using it to power the Arduino via the VIN pin).

---

### 🔁 3. Direction Correction
If one side of the car runs backwards even though the code is correct, you do **not** need to rewire immediately.

In `main.cpp`, change either of these flags from `false` to `true`:
- `invertLeftMotorDirection`
- `invertRightMotorDirection`

That lets you correct reversed motor orientation in code.
