#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
//  CONFIG.h  --  Central tuning file for the whole firmware
// ============================================================================
//  Change values HERE only - every module picks them up automatically.
//  Pin assignments live separately in PINS_CONFIG.h.
//
//  Usage: each module header includes this file, so the constants are
//  available in every translation unit (.ino / .cpp).
// ============================================================================

// ----------------------------------------------------------------------------
// SYSTEM
// ----------------------------------------------------------------------------
#define SYSTEM_LOOP_DELAY_MS     10        // pause at the end of loop() [ms]

// ----------------------------------------------------------------------------
// DISPLAY - 16x2 I2C LCD (LiquidCrystal_I2C)
// ----------------------------------------------------------------------------
#define LCD_I2C_ADDR             0x27      // I2C backpack address (0x3F common)
#define LCD_COLS                 16        // characters per row
#define LCD_ROWS                 2         // number of rows

// ----------------------------------------------------------------------------
// BUTTONS - front-panel controls (START / FORCE STOP)
// ----------------------------------------------------------------------------
#define BUTTON_ACTIVE_LOW        true      // true: pressed = LOW (pull-up wiring)
#define DEBOUNCE_DELAY           50        // debounce window [ms]

// ----------------------------------------------------------------------------
// RELAYS - 8-channel relay board (pumps, motors, valves)
// ----------------------------------------------------------------------------
#define RELAY_ACTIVE_LOW         true      // true: LOW energizes the relay

// ----------------------------------------------------------------------------
// FLOW SENSORS - YF-S201 hall-effect pulse meters
// ----------------------------------------------------------------------------
#define FLOW_CALIBRATION_FACTOR  450.0     // pulses per liter
                                             // (YF-S201 ~450, YF-S401 ~5880)
#define FLOW_MEASURE_INTERVAL    250       // volume/flow update window [ms]
                                             // shorter = tighter fill cutoff
                                             // (one pulse is already 2.22 mL
                                             // with the YF-S201)

// The flow sensors sit on the fill (pump) path only. There is no sensor on the
// outlet valve path, so dispensing is ended by the operator (STOP button).

// ----------------------------------------------------------------------------
// CYCLE PARAMETERS - per treatment line
// ----------------------------------------------------------------------------
#define VOLUME_SANTOL            100.0     // target fill volume, Santol  [mL]
#define VOLUME_SAMPALOK          100.0     // target fill volume, Sampalok [mL]
#define STIRRING_TIME            (15UL * 60UL * 1000UL)  // stir duration [ms]
                                             // (default 15 minutes)

// ----------------------------------------------------------------------------
// CYCLE SAFETY LIMITS - keep the automatic sequence from hanging
// ----------------------------------------------------------------------------
#define FILL_TIMEOUT_MS          (5UL * 60UL * 1000UL)
                                             // abort filling if the target
                                             // volume is not reached [ms]
// Note: the dispensing phase has no flow sensor and is ended by the operator
// with the STOP button, so it has no automatic termination.

#endif // CONFIG_H
