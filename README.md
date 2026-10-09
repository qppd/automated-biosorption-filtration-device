# Automated Biosorption Filtration Device

Semi-automated dual-line filtration apparatus for evaluating the biosorption capacity of Santol (*Sandoricum koetjape*) and Sampalok (*Tamarindus indica*) leaf preparations on methylene blue dye. Control is provided by an ESP32-WROOM-32 development board (38-pin) running Arduino firmware.

## Overview

Two independent treatment lines share one controller. Each line meters a programmed volume of dye solution into a biosorption chamber, stirs the mixture for a fixed contact period, filters the effluent through a leaf-bed filter, and collects the treated liquid in a labeled container. A 16x2 I2C LCD and four push buttons form the operator interface.

The lines are hydraulically and electronically separate and are driven by identical timing constants, so both biosorbents experience the same experimental conditions. This supports a direct comparison of dye removal performance between the two materials.

## Process sequence

Each line executes four states:

1. **Fill** - the peristaltic pump runs until the flow sensor reports the target volume.
2. **Stir** - the impeller motor runs for the configured contact time (default 15 minutes).
3. **Filter** - the outlet valve opens and stays open until flow drops below the configured threshold.
4. **Complete** - the line relays are off; the line stays idle until a new cycle is started.

## Objectives

* Deliver a repeatable, programmable volume of dye solution to each chamber.
* Hold a fixed contact time with consistent stirring.
* Keep the two treatment lines free of cross-contamination.
* Collect independent effluent samples for UV-Vis spectrophotometry.
* Reduce manual operation during a run.

## Hardware

| Subsystem | Components | Function |
|---|---|---|
| Reservoirs | 2 tanks, top-mounted | Store untreated dye solution, one per line |
| Pumps | 2 peristaltic pumps, 12 V | Meter the fill volume into each chamber |
| Flow sensors | 2 YF-S201 hall-effect meters | Pulse output for volume and flow-rate measurement |
| Biosorption chambers | 2 acrylic chambers with leaf holder | Contact zone between dye solution and biosorbent |
| Stirrers | 2 DC gear motors with impeller | Mix the suspension during the contact period |
| Outlet valves | 2 solenoid valves | Release chamber contents to the filter |
| Filtration units | 2 filter housings | Retain leaf particles, pass treated liquid |
| Output containers | 2 labeled vessels | Collect effluent for analysis |
| Control panel | 16x2 I2C LCD, 4 tactile buttons | Status display and manual input |
| Controller | ESP32-WROOM-32, 38-pin board | Firmware execution and I/O control |
| Relay board | 8-channel, active-low | Switches pumps, motors, and valves |
| Power | 12 V DC supply; USB for the ESP32 | Powers loads and controller |
| Frame | Wood frame, acrylic fluid path, rear electronics box | Mechanical support and protection |

## Firmware

Arduino C++ organized as one sketch plus paired module headers and implementations:

| File | Contents |
|---|---|
| `BiosorptionFiltration.ino` | Entry point: `setup()` and `loop()` |
| `PINS_CONFIG.h` | All GPIO assignments |
| `CONFIG.h` | All operating constants |
| `LCD_MODULE.h / .cpp` | I2C LCD driver and text helpers |
| `RELAY_MODULE.h / .cpp` | 8-channel relay control |
| `BUTTON_MODULE.h / .cpp` | Debounced button input |
| `FLOW_SENSOR_MODULE.h / .cpp` | Pulse counting and flow calculation |
| `BIOSORPTION_STATE_MACHINE.h / .cpp` | Per-line cycle control |

Libraries: `LiquidCrystal_I2C` (Frank de Brabander version) and `Wire`. Flow sensing uses the ESP32 core's `attachInterrupt`; no external interrupt library is required.

Module reference, pin map, and constant list: [docs/FIRMWARE.md](docs/FIRMWARE.md). Hardware and software stack: [docs/STACKS.md](docs/STACKS.md).

## Repository layout

```
automated-biosorption-filtration-device/
├─ src/
│  └─ BiosorptionFiltration/
│     ├─ BiosorptionFiltration.ino
│     ├─ PINS_CONFIG.h
│     ├─ CONFIG.h
│     ├─ LCD_MODULE.h / .cpp
│     ├─ RELAY_MODULE.h / .cpp
│     ├─ BUTTON_MODULE.h / .cpp
│     ├─ FLOW_SENSOR_MODULE.h / .cpp
│     └─ BIOSORPTION_STATE_MACHINE.h / .cpp
├─ docs/
│  ├─ FIRMWARE.md
│  └─ STACKS.md
└─ README.md
```

## Getting started

### Requirements

* Arduino IDE 2.x or PlatformIO
* ESP32 board package ("esp32" by Espressif Systems)
* `LiquidCrystal_I2C` library (Frank de Brabander)

### Build and upload

1. Open `src/BiosorptionFiltration/BiosorptionFiltration.ino` in the Arduino IDE.
2. Select the board: **Tools > Board > esp32 > ESP32 Dev Module**.
3. Select the serial port.
4. Compile and upload.

The IDE warning that `LiquidCrystal_I2C` claims support for all architectures is informational and does not affect the build.

### Configuration

Pin assignments are held in one file, operating constants in a second. Both are included by the module headers, so changes require no other edits.

Pins (`PINS_CONFIG.h`):

| Peripheral | GPIOs |
|---|---|
| I2C LCD | SDA 21, SCL 22 |
| Relays 1 to 8 | 4, 13, 14, 16, 17, 18, 19, 23 |
| Flow sensors | 26 (Santol), 27 (Sampalok) |
| Buttons | 32 (Up), 33 (Down), 25 (Select), 5 (Back) |

Constants (`CONFIG.h`):

| Constant | Default | Meaning |
|---|---|---|
| `VOLUME_SANTOL`, `VOLUME_SAMPALOK` | 100.0 mL | Fill target per line |
| `STIRRING_TIME` | 900000 ms | Contact period |
| `FLOW_CALIBRATION_FACTOR` | 450.0 | Pulses per liter |
| `FLOW_MEASURE_INTERVAL` | 1000 ms | Flow recompute window |
| `FILTER_END_FLOW_ML_MIN` | 1.0 mL/min | End of filtration threshold |
| `LCD_I2C_ADDR` | 0x27 | LCD backpack address |
| `DEBOUNCE_DELAY` | 50 ms | Button debounce window |
| `RELAY_ACTIVE_LOW`, `BUTTON_ACTIVE_LOW` | true | Input/output active levels |

### Wiring

No wiring diagram is included in the repository; the pin tables above and in `PINS_CONFIG.h` are the reference for connections. All ESP32 pins are 3.3 V logic - do not apply 5 V to any GPIO. Strapping, UART0, and flash GPIOs are avoided by the pin map, so the board boots normally and the assignment leaves the Wi-Fi section free for later use.

## Operation

1. Connect the ESP32 over USB and switch on the 12 V supply for pumps, motors, and valves.
2. At startup the LCD shows `Biosorption System` on the top row and `Ready` on the bottom row.
3. Press **Select** to start both lines. During a run the top row shows the Santol state and the bottom row the Sampalok state.
4. Collect the effluent from each output container after the corresponding line reports complete.

Fill volumes and timings are compile-time settings in `CONFIG.h`; they cannot be changed from the button interface. The Up, Down, and Back buttons are reserved and are not acted on by the current firmware.

## License

This project is released under the MIT License.
