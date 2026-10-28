# Firmware Reference

Firmware for the Automated Biosorption Filtration Device. Target: ESP32-WROOM-32 development board (38-pin), Arduino core for ESP32, built with the Arduino IDE.

## Project structure

```
src/BiosorptionFiltration/
├── BiosorptionFiltration.ino         entry point (setup / loop)
├── PINS_CONFIG.h                     all GPIO assignments
├── CONFIG.h                          all operating constants
├── LCD_MODULE.h / .cpp               16x2 I2C display driver
├── RELAY_MODULE.h / .cpp             8-channel relay control
├── BUTTON_MODULE.h / .cpp            debounced button input
├── FLOW_SENSOR_MODULE.h / .cpp       pulse flow sensors
└── BIOSORPTION_STATE_MACHINE.h/.cpp  per-line cycle control
```

Every module header includes `PINS_CONFIG.h` or `CONFIG.h`, so a constant or pin is defined in exactly one place.

## Configuration

### PINS_CONFIG.h

| Peripheral | Signal | GPIO |
|---|---|---|
| I2C LCD | SDA | 21 |
| I2C LCD | SCL | 22 |
| Relay 1 | Santol pump | 4 |
| Relay 2 | Sampalok pump | 13 |
| Relay 3 | Santol stirrer | 14 |
| Relay 4 | Sampalok stirrer | 16 |
| Relay 5 | Santol outlet valve | 17 |
| Relay 6 | Sampalok outlet valve | 18 |
| Relay 7 | Reserved | 19 |
| Relay 8 | Reserved | 23 |
| Flow sensor | Santol | 26 |
| Flow sensor | Sampalok | 27 |
| Button | Start | 25 |
| Button | Stop (force stop) | 32 |

Pin groups excluded by the map, and why:

| Group | Pins | Reason |
|---|---|---|
| Boot strapping | 0, 2, 12, 15 | Level at reset selects boot mode or flash voltage |
| UART0 | 1, 3 | USB serial and boot log |
| SPI flash | 6 to 11 | Connected to the module flash, not usable |
| Input only | 34 to 39 | No internal pull-ups, cannot drive outputs |
| I2C bus | 21, 22 | Reserved for the LCD |

GPIO 5 is a strapping pin and is no longer used by the two-button interface; it is left free.

The firmware does not enable Wi-Fi. None of the assigned pins is used by the radio, so Wi-Fi can be enabled later without changing the pin map.

### CONFIG.h

| Constant | Default | Description |
|---|---|---|
| `SYSTEM_LOOP_DELAY_MS` | 10 | Delay at the end of `loop()` |
| `LCD_I2C_ADDR` | 0x27 | LCD backpack address (0x3F is common) |
| `LCD_COLS`, `LCD_ROWS` | 16, 2 | Display geometry |
| `BUTTON_ACTIVE_LOW` | true | Button reads low when pressed |
| `DEBOUNCE_DELAY` | 50 | Debounce window, ms |
| `RELAY_ACTIVE_LOW` | true | Relay energizes on low output |
| `FLOW_CALIBRATION_FACTOR` | 450.0 | Pulses per liter (YF-S201 approx. 450) |
| `FLOW_MEASURE_INTERVAL` | 250 | Volume/flow update window, ms |
| `VOLUME_SANTOL` | 100.0 | Fill target, Santol line, mL |
| `VOLUME_SAMPALOK` | 100.0 | Fill target, Sampalok line, mL |
| `STIRRING_TIME` | 900000 | Contact period, ms (15 minutes) |
| `FILL_TIMEOUT_MS` | 300000 | Abort filling if the target volume is not reached, ms (5 minutes) |

The flow sensors sit on the fill (pump) path only. No sensor measures the outlet,
so dispensing has no automatic termination.

## Entry point

`BiosorptionFiltration.ino`

`setup()` runs in this order:

1. `Serial.begin(115200)` - fault and phase logging (optional to monitor).
2. `lcd_init()` - I2C bus and display.
3. `relay_init()` - all relay channels to the off state.
4. `button_init()` - button inputs with internal pull-ups.
5. `system_init()` - flow sensors, interrupts, state machines, `Press START` screen.

`loop()` calls `button_update()` for debouncing, `system_run()` for the state machines, then waits `SYSTEM_LOOP_DELAY_MS`.

## Modules

### LCD_MODULE

Drives a 16x2 `LiquidCrystal_I2C` display. The LCD object is defined in `LCD_MODULE.cpp` and declared `extern` in the header, so the header is safe to include from any translation unit.

* `lcd_init()` - starts the I2C bus on `I2C_SDA_PIN` / `I2C_SCL_PIN`, initializes the display, turns on the backlight, clears the screen.
* `lcd_display_message(const char* message, uint8_t row = 0)` - writes a message and pads the rest of the row with spaces.
* `lcd_clear()` - clears the display.
* `lcd_write_row(uint8_t row, const char* text)` - writes a row only when its text changed, so live values (metered volume, countdown) update without flicker or redundant I2C traffic.
* `setLCDText(value, x, y)` - writes at a cursor position and pads the rest of the row. Overloads exist for `String`, `int`, `long`, `float`, and `double` (optional decimal count, default 2). Out-of-range coordinates are clamped to the display.

### RELAY_MODULE

Controls the 8-channel relay board.

| Index | Enumeration | Load |
|---|---|---|
| 0 | `RELAY_PUMP_SANTOL` | Santol pump |
| 1 | `RELAY_PUMP_SAMPALOK` | Sampalok pump |
| 2 | `RELAY_MOTOR_SANTOL` | Santol stirrer |
| 3 | `RELAY_MOTOR_SAMPALOK` | Sampalok stirrer |
| 4 | `RELAY_VALVE_SANTOL` | Santol outlet valve |
| 5 | `RELAY_VALVE_SAMPALOK` | Sampalok outlet valve |
| 6 | `RELAY_RESERVED1` | Reserved |
| 7 | `RELAY_RESERVED2` | Reserved |

* `relay_init()` - sets every channel to output and to the off state.
* `relay_on(RelayIndex)`, `relay_off(RelayIndex)` - operate one channel.
* `relay_all_off()` - switches every channel off.

Off state and active level follow `RELAY_ACTIVE_LOW` (default true: the output is low when the relay is energized).

### BUTTON_MODULE

Two buttons with internal pull-ups and software debounce.

| Enumeration | GPIO | Position |
|---|---|---|
| `BTN_START` | 25 | Start |
| `BTN_STOP` | 32 | Force stop |

* `button_init()` - configures the input pins.
* `button_update()` - reads and debounces all buttons; call once per `loop()`.
* `button_pressed(id)`, `button_released(id)`, `button_state(id)` - query one button.
* `button_get_pressed()`, `button_get_released()` - return the button that just changed, or `BTN_NONE`.

`BTN_START` runs the automatic cycle; `BTN_STOP` forces every relay off and returns both lines to `IDLE`, clearing any fault. Because the sequence is automatic, no navigation buttons are needed.

### FLOW_SENSOR_MODULE

Two YF-S201 sensors counted on rising-edge interrupts attached in `system_init()`. Every `FLOW_MEASURE_INTERVAL` window, `flow_sensor_update()` converts the pulse count to volume and flow. Volume is accumulated directly from pulses (one pulse = 1000 / `FLOW_CALIBRATION_FACTOR` mL), so it does not depend on loop timing. Both sensors are on the fill (pump) path; the outlet valve path is not instrumented.

* `flow_sensor_init(FlowSensorData*, pin)` - resets the structure and configures the input.
* `flow_sensor_isr(FlowSensorData*)` - increments the pulse counter; called from the interrupt service routine.
* `flow_sensor_update(FlowSensorData*)` - recomputes flow and volume; call once per `loop()`.
* `flow_sensor_get_flow_rate_ml_min()`, `flow_sensor_get_flow_rate_l_hr()` - current flow.
* `flow_sensor_get_total_volume_ml()`, `flow_sensor_get_total_volume_l()` - accumulated volume.
* `flow_sensor_reset_total_volume()` - clears the accumulator at the start of a cycle.
* `flow_sensor_is_active()` - true after the first measurement window.

Conversion: `FLOW_CALIBRATION_FACTOR` pulses per litre. Volume is summed from the
pulse count, so one pulse is `1000 / FLOW_CALIBRATION_FACTOR` mL (2.22 mL with the
YF-S201). That value is the physical limit of the fill accuracy; a higher
resolution sensor such as the YF-S401 reduces it.

### BIOSORPTION_STATE_MACHINE

One state machine instance per line; both run from the same functions in `system_run()`.

States:

```
IDLE
  |  START pressed, both lines idle or complete
  v
FILLING     pump on, valve closed, motor off
  |  total volume >= VOLUME_x          -> STIRRING
  |  FILL_TIMEOUT_MS elapsed           -> FAULT
  v
STIRRING    motor on, pump off
  |  elapsed >= STIRRING_TIME
  v
DISPENSING  valve open, pump off, motor off
  |  STOP pressed by the operator      -> COMPLETE
  v
COMPLETE    line idle until START is pressed again

FAULT       all line relays off; STOP clears it back to IDLE
```

Dispensing has no automatic termination: the outlet path has no flow sensor, so
the LCD shows `SA:OPEN SP:OPEN` / `Press STOP` and the operator presses STOP once
the product container is full.

* `system_init()` - resets states, initializes both flow sensors, attaches the rising-edge interrupts, clears volume totals, switches all relays off, writes the `Press START` screen.
* `system_run()` - called every `loop()`; updates flow readings, handles the buttons, runs both state machines, then redraws the display.
* `start_santol_cycle()`, `start_sampalok_cycle()` - clear the volume total, start the pump, close the valve, stop the motor, and enter `FILLING`. Accepted from `IDLE` or `COMPLETE`.
* `stop_cycle()` - hard stop: all relays off, both lines to `STATE_IDLE`, faults cleared.

Each line is described by a `Line` record (LCD tag, fill target, phase start
time, relay indices, flow sensor), so both lines run the same code. The current
states are available as `state_santol` and `state_sampalok`.

## LCD layout

The display is 16x2, so shared states get a combined screen and mixed states get
one row per line.

| Screen | Shown when |
|---|---|
| `Biosorption` / `Press START` | Both lines idle |
| `SA:P ON 45.2mL` | Santol filling: pump state and metered volume |
| `SA:M ON 14:32` | Santol stirring: stirrer state and remaining mm:ss |
| `SA:VLV OPEN` | Santol valve open while the other line is still stirring |
| `SA:OPEN SP:OPEN` / `Press STOP` | Dispensing: valves open, operator must stop |
| `SA:DONE SP:DONE` / `Press START` | Both lines complete |
| `SA:TIMEOUT` | Fill safety timeout on that line |

`SA` = Santol, `SP` = Sampalok, `P` = peristaltic pump, `M` = stirrer motor.

## Safety status and limitations

* `BTN_STOP` has two roles: during filling or stirring it aborts the cycle and returns both lines to `IDLE`; during dispensing it closes the valves and marks the batch `COMPLETE`.
* `BTN_STOP` is checked before the state machines, so it takes effect in the same loop pass.
* Filling has a timeout (`FILL_TIMEOUT_MS`): a dead sensor, empty reservoir or stalled pump ends in `FAULT` with the line relays off instead of pumping indefinitely.
* Dispensing has **no** timeout. The outlet is not instrumented, so if the operator does not press STOP the valves stay open and their solenoids stay energized. Add a maximum dispense time if the device may be left unattended.
* A faulted line must be cleared with `BTN_STOP` before a new cycle can start.
* The sequence is repeatable: `START` is accepted again once both lines are `COMPLETE`.
* No hardware watchdog timer is enabled.
* Serial output at 115200 baud logs phase changes, faults and stops; it is not required for operation.
* Wi-Fi and Bluetooth are unused.
