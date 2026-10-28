# Automated Biosorption Filtration Device

Automated dual-line filtration apparatus for evaluating the biosorption capacity of Santol (*Sandoricum koetjape*) and Sampalok (*Tamarindus indica*) leaf preparations on methylene blue dye. Control is provided by an ESP32-WROOM-32 development board (38-pin) running Arduino firmware.

## Overview

Two independent treatment lines share one controller. Each line meters a programmed volume of dye solution into a biosorption chamber, stirs the mixture for a fixed contact period, filters the effluent through a leaf-bed filter, and collects the treated liquid in a labeled container. A 16x2 I2C LCD and two push buttons (START and FORCE STOP) form the operator interface.

The lines are hydraulically and electronically separate and are driven by identical timing constants, so both biosorbents experience the same experimental conditions. This supports a direct comparison of dye removal performance between the two materials.

## Process sequence

Each line executes the same sequence:

1. **Fill** - the peristaltic pump runs until the flow sensor reports the target volume.
2. **Stir** - the impeller motor runs for the configured contact time (default 15 minutes).
3. **Dispense** - the outlet solenoid valve opens. There is no flow sensor on this path, so the valve stays open until the operator presses **FORCE STOP** once the product is fully dispensed.
4. **Complete** - the line relays are off; the line waits for START to begin another cycle.

Both lines run together from a single START press. Filling has a safety timeout:
if the target volume is not reached the line enters a **Fault** state with its
relays off instead of pumping indefinitely. FORCE STOP also aborts a running
fill or stir at any time.

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
| Control panel | 16x2 I2C LCD, 2 tactile buttons | Status display, START and FORCE STOP |
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

* Arduino IDE 2.x
* ESP32 board package ("esp32" by Espressif Systems)
* `LiquidCrystal_I2C` library by **Frank de Brabander**

### Build and upload

1. Install the ESP32 board package: **Tools > Board > Boards Manager**, search `esp32`, install **esp32 by Espressif Systems**.
2. Install the LCD library: **Tools > Manage Libraries**, search `LiquidCrystal I2C` and install the one by **Frank de Brabander**. Do not install the similarly named library by Marco Schwartz: it uses `begin()` instead of `init()` and will not compile with this sketch.
3. Open `src/BiosorptionFiltration/BiosorptionFiltration.ino` in the Arduino IDE. The sketch folder name must stay `BiosorptionFiltration`; the IDE compiles every `.cpp` and `.h` file beside the `.ino` automatically.
4. Select the board: **Tools > Board > esp32 > ESP32 Dev Module**.
5. Select the serial port.
6. Compile and upload.

The IDE warning that `LiquidCrystal_I2C` claims support for all architectures is informational and does not affect the build.

### Configuration

Pin assignments are held in one file, operating constants in a second. Both are included by the module headers, so changes require no other edits.

Pins (`PINS_CONFIG.h`):

| Peripheral | GPIOs |
|---|---|
| I2C LCD | SDA 21, SCL 22 |
| Relays 1 to 8 | 4, 13, 14, 16, 17, 18, 19, 23 |
| Flow sensors | 26 (Santol), 27 (Sampalok) |
| Buttons | 25 (START), 32 (FORCE STOP) |

Constants (`CONFIG.h`):

| Constant | Default | Meaning |
|---|---|---|
| `VOLUME_SANTOL`, `VOLUME_SAMPALOK` | 100.0 mL | Fill target per line |
| `STIRRING_TIME` | 900000 ms | Contact period |
| `FILL_TIMEOUT_MS` | 300000 ms | Abort filling if the target volume is not reached |
| `FLOW_CALIBRATION_FACTOR` | 450.0 | Pulses per liter |
| `FLOW_MEASURE_INTERVAL` | 250 ms | Volume/flow update window |
| `LCD_I2C_ADDR` | 0x27 | LCD backpack address |
| `DEBOUNCE_DELAY` | 50 ms | Button debounce window |
| `RELAY_ACTIVE_LOW`, `BUTTON_ACTIVE_LOW` | true | Input/output active levels |

The flow sensors are on the **fill path only**. Nothing measures the outlet, so the
dispensing phase cannot detect when the chamber is empty and is ended by the
operator with FORCE STOP.

### Wiring

No wiring diagram is included in the repository; the pin tables above and in `PINS_CONFIG.h` are the reference for connections. All ESP32 pins are 3.3 V logic - do not apply 5 V to any GPIO. Strapping, UART0, and flash GPIOs are avoided by the pin map, so the board boots normally and the assignment leaves the Wi-Fi section free for later use.

## Operation

1. Connect the ESP32 over USB and switch on the 12 V supply for pumps, motors, and valves.
2. At startup the LCD shows `Biosorption` / `Press START`.
3. Press **START** to run both lines.
4. During filling each row shows the pump state and the volume its flow sensor has metered, e.g. `SA:P ON 45.2mL`.
5. During stirring each row shows the stirrer state and the remaining contact time, e.g. `SA:M ON 14:32`.
6. When the contact time is over, both valves open and the LCD shows `SA:OPEN SP:OPEN` / `Press STOP`.
7. Watch the product container. When it is full, press **FORCE STOP**: the valves close and the LCD shows `SA:DONE SP:DONE` / `Press START`.
8. Collect the effluent from each output container and press **START** for the next batch.

Pressing **FORCE STOP** during filling or stirring aborts the cycle instead and returns both lines to idle.

A line that shows `TIMEOUT` did not reach its fill target in time and stopped with its relays off. Check the pump, reservoir and flow sensor, then clear it with **FORCE STOP**.

## LCD screens

| Screen | Meaning |
|---|---|
| `Biosorption` / `Press START` | Idle, ready for a cycle |
| `SA:P ON 45.2mL` | Santol pump running, 45.2 mL metered by its flow sensor |
| `SA:M ON 14:32` | Santol stirrer running, 14 min 32 s of contact time left |
| `SA:VLV OPEN` | Santol valve open while the other line is still stirring |
| `SA:OPEN SP:OPEN` / `Press STOP` | Both valves open, dispensing |
| `SA:DONE SP:DONE` / `Press START` | Batch finished, ready for another |
| `SA:TIMEOUT` | Fill safety timeout on that line |

`SA` is Santol, `SP` is Sampalok, `P` is the peristaltic pump and `M` the stirrer motor. Fill volumes and timings are compile-time settings in `CONFIG.h`; they cannot be changed from the button interface. Phase changes, faults and force stops are also logged on the serial port at 115200 baud.

## License

This project is released under the MIT License.
