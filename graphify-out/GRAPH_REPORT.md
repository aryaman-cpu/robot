# Graph Report - .  (2026-04-28)

## Corpus Check
- 11 files · ~65,631 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 66 nodes · 98 edges · 12 communities detected
- Extraction: 81% EXTRACTED · 16% INFERRED · 3% AMBIGUOUS · INFERRED: 16 edges (avg confidence: 0.86)
- Token cost: 450 input · 350 output

## Community Hubs (Navigation)
- [[_COMMUNITY_Serial Command Parsing|Serial Command Parsing]]
- [[_COMMUNITY_Main Control Loop & Fail-Safe|Main Control Loop & Fail-Safe]]
- [[_COMMUNITY_Drive Output & Motor Control|Drive Output & Motor Control]]
- [[_COMMUNITY_AGENTS.md Rules & Guidelines|AGENTS.md Rules & Guidelines]]
- [[_COMMUNITY_Robot BOM (build v1)|Robot BOM (build v1)]]
- [[_COMMUNITY_Robot BOM (build v2)|Robot BOM (build v2)]]
- [[_COMMUNITY_Robot BOM (build v3)|Robot BOM (build v3)]]
- [[_COMMUNITY_Robot BOM (build v4)|Robot BOM (build v4)]]
- [[_COMMUNITY_Robot BOM (build v5)|Robot BOM (build v5)]]
- [[_COMMUNITY_Robot Electronics Assembly|Robot Electronics Assembly]]
- [[_COMMUNITY_HC-05 Wiring & Serial Selection|HC-05 Wiring & Serial Selection]]
- [[_COMMUNITY_L298N Motor Driver Wiring|L298N Motor Driver Wiring]]

## God Nodes (most connected - your core abstractions)
1. `executeDriveCommand()` - 7 edges
2. `readNextCommand()` - 6 edges
3. `checkBluetoothCommand()` - 5 edges
4. `handleIncomingByte()` - 5 edges
5. `setDriveOutputs()` - 5 edges
6. `Robot Electronics Photo` - 5 edges
7. `Robot Control Assembly` - 5 edges
8. `L298N Motor Driver Module` - 5 edges
9. `checkFailSafe()` - 4 edges
10. `debugPrintln()` - 4 edges

## Surprising Connections (you probably didn't know these)
- `checkBluetoothCommand()` --calls--> `readNextCommand()`  [EXTRACTED]
  main.cpp → main.cpp  _Bridges community 1 → community 0_
- `checkBluetoothCommand()` --calls--> `executeDriveCommand()`  [EXTRACTED]
  main.cpp → main.cpp  _Bridges community 1 → community 2_

## Hyperedges (group relationships)
- **Competition Safety Pattern** — agents_fail_safe_constraint, agents_non_blocking_logic, agents_safety_first_initialization, main_fail_safe_logic [INFERRED 0.89]
- **Robot Car Control Stack** — 195e8434_arduino_uno, 195e8434_l298n_motor_driver, 195e8434_bluetooth_module, 195e8434_battery_pack, 195e8434_robot_car_chassis [INFERRED 0.93]
- **Robot Drive Platform** — cb75d34f-fecb-442d-9f1b-ec1fd56060b4_robot_chassis, cb75d34f-fecb-442d-9f1b-ec1fd56060b4_arduino_uno, cb75d34f-fecb-442d-9f1b-ec1fd56060b4_l298n_motor_driver, cb75d34f-fecb-442d-9f1b-ec1fd56060b4_battery_pack, cb75d34f-fecb-442d-9f1b-ec1fd56060b4_bluetooth_module [INFERRED 0.95]
- **Robot Control Stack** — 41d3fd0e-9b44-405c-b9d0-97134b20700e_arduino_uno, 41d3fd0e-9b44-405c-b9d0-97134b20700e_bluetooth_module, 41d3fd0e-9b44-405c-b9d0-97134b20700e_motor_driver, 41d3fd0e-9b44-405c-b9d0-97134b20700e_wheel_chassis, 41d3fd0e-9b44-405c-b9d0-97134b20700e_battery_pack [INFERRED 0.90]
- **Robot Control Assembly** — c0b9bbe1_arduino_uno, c0b9bbe1_l298n_motor_driver, c0b9bbe1_battery_pack, c0b9bbe1_wire_harness, c0b9bbe1_robot_chassis [INFERRED 0.91]
- **Drivetrain Subsystem** — c0b9bbe1_l298n_motor_driver, c0b9bbe1_robot_wheel, c0b9bbe1_robot_chassis [INFERRED 0.82]
- **Robot Drive Assembly** — a4a7a9cd_robot_chassis, a4a7a9cd_dc_gear_motors, a4a7a9cd_motor_driver_module, a4a7a9cd_arduino_uno, a4a7a9cd_battery_pack, a4a7a9cd_wiring_harness [EXTRACTED 0.96]
- **Robot Control Stack Assembly** — d4ebd8a5-df84-4355-b279-cb4c832d7a33_arduino_uno, d4ebd8a5-df84-4355-b279-cb4c832d7a33_bluetooth_module, d4ebd8a5-df84-4355-b279-cb4c832d7a33_l298n_motor_driver, d4ebd8a5-df84-4355-b279-cb4c832d7a33_wired_power_and_signal_links [EXTRACTED 0.95]

## Communities

### Community 0 - "Serial Command Parsing"
Cohesion: 0.62
Nodes (5): clearCommandBuffer(), decodeCommandToken(), handleIncomingByte(), isValidDriveCommand(), readNextCommand()

### Community 1 - "Main Control Loop & Fail-Safe"
Cohesion: 0.47
Nodes (6): checkBluetoothCommand(), checkFailSafe(), debugPrintln(), loop(), setup(), stopDriveMotors()

### Community 2 - "Drive Output & Motor Control"
Cohesion: 0.53
Nodes (6): executeDriveCommand(), moveBackward(), moveForward(), setDriveOutputs(), turnLeft(), turnRight()

### Community 3 - "AGENTS.md Rules & Guidelines"
Cohesion: 0.33
Nodes (6): Antigravity Robot, Locomotion Weapon and Fail-Safe Objective, Mandatory Fail-Safe Constraint, Interrupt-Based RC Signal Reading, Non-Blocking millis Logic, Safety-First Pin Initialization

### Community 4 - "Robot BOM (build v1)"
Cohesion: 0.6
Nodes (6): Arduino Uno, Battery Pack, Bluetooth Serial Module, L298N Motor Driver Module, Four-Wheel Robot Chassis, Robot Electronics Photo

### Community 5 - "Robot BOM (build v2)"
Cohesion: 0.67
Nodes (6): Arduino Uno, Battery Pack, Bluetooth Serial Module, L298N Motor Driver Module, Robot Control Assembly, Wheeled Robot Chassis

### Community 6 - "Robot BOM (build v3)"
Cohesion: 0.47
Nodes (6): Arduino Uno, Battery Pack, L298N Motor Driver Module, Robot Chassis, Robot Wheel, Wire Harness

### Community 7 - "Robot BOM (build v4)"
Cohesion: 0.4
Nodes (6): Arduino Uno, Battery Pack, DC Gear Motors, Motor Driver Module, Wheeled Robot Chassis, Wiring Harness

### Community 8 - "Robot BOM (build v5)"
Cohesion: 0.5
Nodes (5): Arduino Uno, Battery Pack, Bluetooth Serial Module, L298N Motor Driver Module, Four-Wheel Robot Car Chassis

### Community 9 - "Robot Electronics Assembly"
Cohesion: 0.8
Nodes (5): Arduino Uno, Bluetooth Module, L298N Motor Driver, Robot Electronics Assembly, Wired Power and Signal Links

### Community 10 - "HC-05 Wiring & Serial Selection"
Cohesion: 0.5
Nodes (4): Hardware Serial (D0/D1), HC-05 Bluetooth Module, SoftwareSerial Fallback (D2/D3), Voltage Divider (3.3V Logic)

### Community 11 - "L298N Motor Driver Wiring"
Cohesion: 0.67
Nodes (3): Direction Correction Flags, ENA/ENB Jumper Caps (Always-On), L298N Motor Driver

## Ambiguous Edges - Review These
- `Arduino Uno` → `Bluetooth Serial Module`  [AMBIGUOUS]
  images/cb75d34f-fecb-442d-9f1b-ec1fd56060b4.JPG · relation: semantically_similar_to
- `Arduino Uno` → `Battery Pack`  [AMBIGUOUS]
  images/41d3fd0e-9b44-405c-b9d0-97134b20700e.JPG · relation: shares_data_with
- `L298N Motor Driver Module` → `Battery Pack`  [AMBIGUOUS]
  images/c0b9bbe1-0097-48fe-92be-15fcd8c6e610.JPG · relation: shares_data_with

## Knowledge Gaps
- **16 isolated node(s):** `Antigravity Robot`, `Non-Blocking millis Logic`, `Interrupt-Based RC Signal Reading`, `Safety-First Pin Initialization`, `Bluetooth Serial Module` (+11 more)
  These have ≤1 connection - possible missing edges or undocumented components.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **What is the exact relationship between `Arduino Uno` and `Bluetooth Serial Module`?**
  _Edge tagged AMBIGUOUS (relation: semantically_similar_to) - confidence is low._
- **What is the exact relationship between `Arduino Uno` and `Battery Pack`?**
  _Edge tagged AMBIGUOUS (relation: shares_data_with) - confidence is low._
- **What is the exact relationship between `L298N Motor Driver Module` and `Battery Pack`?**
  _Edge tagged AMBIGUOUS (relation: shares_data_with) - confidence is low._
- **Why does `executeDriveCommand()` connect `Drive Output & Motor Control` to `Serial Command Parsing`, `Main Control Loop & Fail-Safe`?**
  _High betweenness centrality (0.003) - this node is a cross-community bridge._
- **What connects `Antigravity Robot`, `Non-Blocking millis Logic`, `Interrupt-Based RC Signal Reading` to the rest of the system?**
  _16 weakly-connected nodes found - possible documentation gaps or missing edges._