# Technology Stack

Hardware and software components of the Automated Biosorption Filtration Device, with the role of each component.

## Hardware

| Component | Role | Notes |
|---|---|---|
| ESP32-WROOM-32 dev board (38-pin) | Main controller | 3.3 V logic; pin map in `PINS_CONFIG.h` |
| 16x2 I2C LCD (HD44780 + I2C backpack) | Operator display | Address 0x27 default; SDA 21, SCL 22 |
| 8-channel relay module | Load switching | Active-low; drives pumps, motors, valves |
| YF-S201 flow sensors (2) | Flow measurement | Pulse output on GPIO 26 and 27, interrupt driven |
| Tactile buttons (4) | Operator input | GPIO 32, 33, 25, 5; internal pull-ups |
| Peristaltic pumps (2) | Fluid transfer | 12 V; deliver the programmed fill volume |
| DC gear motors (2) | Mixing | Run the impeller during the contact period |
| Solenoid valves (2) | Outlet control | Open the chamber outlets for filtration |
| Filter housings (2) | Solid separation | Retain leaf particles, pass treated liquid |
| Acrylic chambers (2) | Reaction vessel | Hold the leaf bed and dye solution |
| Reservoirs (2) | Solution storage | Hold untreated dye solution above each line |
| 12 V DC supply | Power | Runs pumps, motors, valves; ESP32 runs from USB |
| Wood frame, acrylic fluid path, electronics box | Structure | Mounts the fluid path and separates it from the electronics |

## Firmware

| Item | Choice | Role |
|---|---|---|
| Framework | Arduino core for ESP32 | Setup/loop, GPIO, timers, interrupts |
| LCD driver | LiquidCrystal_I2C (Frank de Brabander) | Drives the 16x2 display over I2C |
| I2C bus | Wire | Bus between the ESP32 and the LCD backpack |
| Pulse counting | Native `attachInterrupt` | Counts flow sensor pulses outside the main loop |
| Sequencing | Finite state machine, one per line | Fill, stir, filter, complete |
| Input conditioning | Software debounce, 50 ms | Clean button edges |
| Code layout | Paired `.h` / `.cpp` modules | One module per hardware block; central `PINS_CONFIG.h` and `CONFIG.h` |

## Toolchain

| Tool | Role |
|---|---|
| Arduino IDE 2.x or PlatformIO | Editing, compilation, upload |
| ESP32 board package (Espressif) | Board definitions for the ESP32 Dev Module target |
| Git | Version control for firmware, documentation, and hardware notes |
| Markdown | Documentation format for README.md, FIRMWARE.md, and STACKS.md |
| Fritzing or KiCad | Optional: wiring diagrams and PCB layout |

## Interfaces

| Interface | Used by | Notes |
|---|---|---|
| I2C | LCD | SDA 21, SCL 22; two wires only |
| UART0 | Programming and debug | USB connection; the firmware does not transmit logs |
| SPI | Not used | Flash stays on the module; spare GPIOs remain for expansion |
| GPIO | Relays, buttons | Direct control; map in `PINS_CONFIG.h` |
| External interrupts | Flow sensors | Rising edge on GPIO 26 and 27 |

## Power

| Aspect | Implementation |
|---|---|
| Logic level | 3.3 V ESP32; 5 V must not be applied to any GPIO |
| Loads | 12 V for pumps, motors, and valves, switched through the relay board |
| Controller | ESP32 from USB or a regulated 5 V input |
| LCD and relay board | Powered from their rated supply (typically 5 V) |
| Grounding | Common ground between ESP32, relay board, and supply |
| Fusing | Fuse or PTC on the 12 V line for overcurrent protection |

## Configuration

| Parameter | File | Adjustment |
|---|---|---|
| Flow sensor pulses per liter | `CONFIG.h` | `FLOW_CALIBRATION_FACTOR` from the sensor datasheet or a calibration test |
| Target volume per line | `CONFIG.h` | `VOLUME_SANTOL`, `VOLUME_SAMPALOK` in mL |
| Stirring duration | `CONFIG.h` | `STIRRING_TIME` in ms; default 15 minutes |
| Filtration end threshold | `CONFIG.h` | `FILTER_END_FLOW_ML_MIN` in mL/min |
| LCD I2C address | `CONFIG.h` | `LCD_I2C_ADDR` as 0x27 or 0x3F per backpack |
| Debounce delay | `CONFIG.h` | `DEBOUNCE_DELAY` if buttons chatter |
| Relay and button active level | `CONFIG.h` | `RELAY_ACTIVE_LOW`, `BUTTON_ACTIVE_LOW` |
| Pin assignments | `PINS_CONFIG.h` | Single source for every GPIO |

The firmware follows a strict separation between hardware drivers and application logic; each module can be tested on its own. Adjust `PINS_CONFIG.h` to the actual wiring and the constants in `CONFIG.h` to the experiment before compiling.
