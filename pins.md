
### 📡 1. Bluetooth (ESP32 Built-In)

The ESP32 has Bluetooth Classic (SPP) built in — **no external HC-05 module required**.

| Setting | Value | Notes |
| :--- | :--- | :--- |
| **Protocol** | Bluetooth Classic SPP | Compatible with most RC car Bluetooth apps |
| **Device name** | `Antigravity` | Set via `BT_DEVICE_NAME` in `main.cpp` |
| **Disconnect detect** | Automatic callback | `ESP_SPP_CLOSE_EVT` triggers immediate motor stop |

---

### 🏎️ 2. BTS7960 Motor Drivers (×2)

Two BTS7960 43A motor drivers, one per side. Each side drives two motors wired **in parallel**.

#### Left BTS7960 (left-side motors)

| BTS7960 Pin | ESP32 GPIO | Function |
| :--- | :--- | :--- |
| **RPWM** | **GPIO 32** | Forward PWM |
| **LPWM** | **GPIO 33** | Reverse PWM |
| **R_EN** | **GPIO 25** | Right enable (active HIGH) |
| **L_EN** | **GPIO 26** | Left enable (active HIGH) |

#### Right BTS7960 (right-side motors)

| BTS7960 Pin | ESP32 GPIO | Function |
| :--- | :--- | :--- |
| **RPWM** | **GPIO 27** | Forward PWM |
| **LPWM** | **GPIO 14** | Reverse PWM |
| **R_EN** | **GPIO 4** | Right enable (active HIGH) |
| **L_EN** | **GPIO 13** | Left enable (active HIGH) |

**BTS7960 Power:**
*   **B+** / **B−**: Connect to the **12V celled battery** (positive and negative).
*   **VCC** (logic): Connect to **ESP32 3.3V** or the 5V pin on the expansion board if the BTS7960 logic is 5V-tolerant. Check your specific module.
*   **GND**: Common ground with ESP32 and battery.

---

### 💡 3. Status LED

| Function | ESP32 GPIO | Notes |
| :--- | :--- | :--- |
| **Status LED** | **GPIO 2** | On-board LED on most ESP32 dev boards. HIGH = motors driving. |

---

### 🔋 4. Power

| Source | Powers | Notes |
| :--- | :--- | :--- |
| **12V celled battery** | BTS7960 motor drivers → motors | High-current path |
| **LiPo battery** | ESP32 (via expansion board) | Logic power |

**⚠️ Common ground is mandatory.** Connect the GND of the 12V battery, both BTS7960 modules, the ESP32, and the LiPo supply together.

---

### 🔄 5. Direction Correction

If one side of the car runs backwards, change the corresponding flag in `main.cpp`:
- `invertLeftMotorDirection`
- `invertRightMotorDirection`

---

### 📌 6. GPIO Selection Notes

These GPIOs were chosen to avoid ESP32 boot-strapping conflicts:
- **GPIO 0, 2, 5, 15**: Boot-strapping pins (GPIO 2 is used only for the status LED — safe for output after boot).
- **GPIO 6–11**: Connected to internal flash — **never use**.
- **GPIO 12**: Pulls flash voltage high at boot if driven — **avoided**.
- **GPIO 34–39**: Input-only — **not used for motor outputs**.
