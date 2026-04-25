
### 📡 1. HC-05 Bluetooth Module
| HC-05 Pin | Arduino Pin | Notes |
| :--- | :--- | :--- |
| **TX** (Transmit) | **D2** | Sends data to Arduino |
| **RX** (Receive) | **D3** | Receives data from Arduino *(Pro-tip: The HC-05 expects 3.3V on this pin. It's best practice to use a simple resistor voltage divider here, though many people plug it directly into 5V without immediate issues).* |
| **VCC** | **5V** | Power |
| **GND** | **GND** | Ground |

---

### 🏎️ 2. L298N Motor Driver
*Note: Make sure to remove the physical jumper caps on the ENA and ENB pins on the L298N board so you can plug your Arduino wires into them for speed control.*

| L298N Pin | Arduino Pin | Function |
| :--- | :--- | :--- |
| **ENA** (Enable A) | **D5** | Left Motor PWM Speed Control |
| **IN1** | **D7** | Left Motor Forward |
| **IN2** | **D8** | Left Motor Backward |
| **IN3** | **D9** | Right Motor Forward |
| **IN4** | **D10** | Right Motor Backward |
| **ENB** (Enable B) | **D6** | Right Motor PWM Speed Control |

**L298N Power Connections:**
*   **12V Pin:** Connect to your main battery Positive (+).
*   **GND Pin:** Connect to your main battery Negative (-) **AND** connect this to the Arduino's **GND** pin. *(Sharing a common ground between the Arduino and the motor driver is mandatory, or the signals won't work).*
*   **5V Pin:** Leave empty (unless you are using it to power the Arduino via the VIN pin).

---

### ⚔️ 3. Weapon Servo
| Servo Wire Color | Arduino Pin | Function |
| :--- | :--- | :--- |
| **Yellow / Orange / White** | **D11** | Signal / Control |
| **Red** | **5V** | Power *(See warning below)* |
| **Brown / Black** | **GND** | Ground |

**⚠️ Servo Power Warning:**
If you are using a tiny 9g micro-servo for a weapon lock, powering it directly from the Arduino 5V pin is fine. However, if you are using a heavy-duty, high-torque metal gear servo for a flipper or lifter, **do not power it from the Arduino 5V pin**. A heavy servo will draw too much current and cause the Arduino to brown-out (crash/restart), which will trigger your fail-safe constantly. Instead, power heavy servos from a separate 5V regulator (BEC) or a separate battery pack, making sure to connect the grounds together.