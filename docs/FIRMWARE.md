# Firmware Reference

Firmware for the Automated Biosorption Filtration Device. Target: ESP32-WROOM-32 development board (38-pin), Arduino core for ESP32, Arduino IDE or PlatformIO.

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
| Button | Up | 32 |
| Button | Down | 33 |
| Button | Select | 25 |
| Button | Back | 5 |

Pin groups excluded by the map, and why:

| Group | Pins | Reason |
|---|---|---|
| Boot strapping | 0, 2, 12, 15 | Level at reset selects boot mode or flash voltage |
| UART0 | 1, 3 | USB serial and boot log |
| SPI flash | 6 to 11 | Connected to the module flash, not usable |
| Input only | 34 to 39 | No internal pull-ups, cannot drive outputs |
| I2C bus | 21, 22 | Reserved for the LCD |

GPIO 5 is a strapping pin but is used for the Back button only: the internal pull-up holds it high at reset, which does not affect boot.

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
| `FLOW_MEASURE_INTERVAL` | 1000 | Flow recompute window, ms |
| `FILTER_END_FLOW_ML_MIN` | 1.0 | Flow below this ends the filter phase, mL/min |
| `VOLUME_SANTOL` | 100.0 | Fill target, Santol line, mL |
| `VOLUME_SAMPALOK` | 100.0 | Fill target, Sampalok line, mL |
| `STIRRING_TIME` | 900000 | Contact period, ms (15 minutes) |

## Entry point

`BiosorptionFiltration.ino`

`setup()` runs in this order:

1. `lcd_init()` - I2C bus and display.
2. `relay_init()` - all relay channels to the off state.
3. `button_init()` - button inputs with internal pull-ups.
4. `system_init()` - flow sensors, interrupts, state machines.

`loop()` calls `button_update()` for debouncing, `system_run()` for the state machines, then waits `SYSTEM_LOOP_DELAY_MS`.

## Modules

### LCD_MODULE

Drives a 16x2 `LiquidCrystal_I2C` display. The LCD object is defined in `LCD_MODULE.cpp` and declared `extern` in the header, so the header is safe to include from any translation unit.

* `lcd_init()` - starts the I2C bus on `I2C_SDA_PIN` / `I2C_SCL_PIN`, initializes the display, turns on the backlight, clears the screen.
* `lcd_display_message(const char* message, uint8_t row = 0)` - writes a message and pads the rest of the row with spaces.
* `lcd_clear()` - clears the display.
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

Four buttons with internal pull-ups and software debounce.

| Enumeration | GPIO | Position |
|---|---|---|
| `BTN_UP` | 32 | Up |
| `BTN_DOWN` | 33 | Down |
| `BTN_SELECT` | 25 | Select |
| `BTN_BACK` | 5 | Back |

* `button_init()` - configures the input pins.
* `button_update()` - reads and debounces all buttons; call once per `loop()`.
* `button_pressed(id)`, `button_released(id)`, `button_state(id)` - query one button.
* `button_get_pressed()`, `button_get_released()` - return the button that just changed, or `BTN_NONE`.

`BTN_SELECT` starts both lines when both are idle. `BTN_UP`, `BTN_DOWN`, and `BTN_BACK` are read but not acted on; they are reserved for menu functions.

### FLOW_SENSOR_MODULE

Two YF-S201 sensors counted on rising-edge interrupts attached in `system_init()`. Every `FLOW_MEASURE_INTERVAL` window, `flow_sensor_update()` converts the pulse count to a flow rate and accumulates volume.

* `flow_sensor_init(FlowSensorData*, pin)` - resets the structure and configures the input.
* `flow_sensor_isr(FlowSensorData*)` - increments the pulse counter; called from the interrupt service routine.
* `flow_sensor_update(FlowSensorData*)` - recomputes flow and volume; call once per `loop()`.
* `flow_sensor_get_flow_rate_ml_min()`, `flow_sensor_get_flow_rate_l_hr()` - current flow.
* `flow_sensor_get_total_volume_ml()`, `flow_sensor_get_total_volume_l()` - accumulated volume.
* `flow_sensor_reset_total_volume()` - clears the accumulator at the start of a cycle.
* `flow_sensor_is_active()` - true after the first measurement window.

Conversion: `FLOW_CALIBRATION_FACTOR` pulses per liter, measured over `FLOW_MEASURE_INTERVAL`.

### BIOSORPTION_STATE_MACHINE

One state machine instance per line; both run from the same functions in `system_run()`.

States:

```
IDLE
  |  Select pressed, both lines idle
  v
FILLING     pump on, valve closed, motor off
  |  total volume >= VOLUME_x
  v
STIRRING    motor on, pump off
  |  elapsed >= STIRRING_TIME
  v
FILTERING   valve open, pump off, motor off
  |  flow < FILTER_END_FLOW_ML_MIN
  v
COMPLETE    line idle until a new cycle
```

* `system_init()` - resets states, initializes both flow sensors, attaches the rising-edge interrupts, clears volume totals, switches all relays off, writes the initial LCD status.
* `system_run()` - called every `loop()`; updates flow readings, runs both state machines, and handles button input.
* `start_santol_cycle()`, `start_sampalok_cycle()` - clear the volume total, start the pump, close the valve, stop the motor, and enter `FILLING`.
* `stop_cycle()` - emergency stop: all relays off, both lines to `STATE_IDLE`, LCD shows `Stopped`.

The current line states are available as `state_santol` and `state_sampalok`.

## Safety status and limitations

* `stop_cycle()` is implemented but not assigned to any button or other input; there is no operator emergency stop in the current interface.
* No watchdog timer is enabled.
* No fill timeout: a pump that fails to reach the target volume leaves the line in `FILLING` until reset.
* The serial port is not used; there is no runtime logging.
* Wi-Fi and Bluetooth are unused.
* `BTN_UP`, `BTN_DOWN`, and `BTN_BACK` are read but not acted on.
