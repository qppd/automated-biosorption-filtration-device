#ifndef PINS_CONFIG_H
#define PINS_CONFIG_H

// ============================================================================
//  PINS_CONFIG.h  --  Central pin map for the ESP32 38-pin WROOM DevKit
// ============================================================================
//
//  Every GPIO assignment of the project lives here so wiring changes are
//  made in ONE place only. The module headers (RELAY/FLOW/BUTTON/LCD)
//  include this file instead of defining pins themselves.
//
//  PINS DELIBERATELY AVOIDED (boot / WiFi / USB-serial safety):
//  -----------------------------------------------------------------
//  GPIO 0, 2, 12, 15   Boot-strapping pins.
//                       - GPIO0  : LOW = download/boot mode
//                       - GPIO2  : must stay low/floating for serial download
//                       - GPIO12 : MTDI, selects flash voltage (1.8 V/3.3 V);
//                                  driving it HIGH at reset breaks boot
//                       - GPIO15 : MTDO boot strapping
//                       GPIO5 is a strapping pin too (SDIO timing only) and is
//                       used solely for a button with an internal pull-up, so
//                       it idles HIGH at reset -> boot unaffected.
//  GPIO 1, 3           UART0 TX/RX -> USB serial, boot log, flash output.
//  GPIO 6 - 11         Connected to the internal SPI flash (never usable).
//  GPIO 34 - 39        Input-only, NO internal pull-ups (breaks INPUT_PULLUP).
//  GPIO 21, 22         Reserved for the I2C LCD (ESP32 default Wire pins).
//
//  WiFi note: the ESP32 WiFi/RF section does not occupy any of these GPIOs
//  (RF is internal to the module). Keeping strapping pins and flash GPIOs
//  untouched means WiFi can be enabled later without rewiring or boot risk.
//  For best RF performance keep antenna-side pins and module area clear of
//  wiring, and keep relay/pump wiring away from the antenna.
// ============================================================================

// ----------------------------------------------------------------------------
// I2C LCD (16x2, LiquidCrystal_I2C) - ESP32 default Wire pins
// ----------------------------------------------------------------------------
#define I2C_SDA_PIN         21
#define I2C_SCL_PIN         22

// ----------------------------------------------------------------------------
// 8-channel relay module (active LOW)  ->  pumps, motors, valves
// All pins are ordinary output-capable GPIOs, none are boot-strapping pins.
// ----------------------------------------------------------------------------
#define RELAY_1_PIN         4    // Pump 1 - Santol
#define RELAY_2_PIN         13   // Pump 2 - Sampalok
#define RELAY_3_PIN         14   // Motor 1 - Santol stirrer
#define RELAY_4_PIN         16   // Motor 2 - Sampalok stirrer
#define RELAY_5_PIN         17   // Valve 1 - Santol outlet
#define RELAY_6_PIN         18   // Valve 2 - Sampalok outlet
#define RELAY_7_PIN         19   // Reserved / Future
#define RELAY_8_PIN         23   // Reserved / Future

// ----------------------------------------------------------------------------
// YF-S201 flow sensors (interrupt driven, INPUT_PULLUP)
// Every ESP32 GPIO supports interrupts; these avoid boot-strapping and UART0.
// ----------------------------------------------------------------------------
#define FLOW_SENSOR_SANTOL_PIN    26
#define FLOW_SENSOR_SAMPALOK_PIN  27

// ----------------------------------------------------------------------------
// Front-panel buttons (INPUT_PULLUP, active LOW)
// Must be ordinary GPIOs - input-only 34-39 have no internal pull-ups.
// ----------------------------------------------------------------------------
#define BTN_UP_PIN          32
#define BTN_DOWN_PIN        33
#define BTN_SELECT_PIN      25
#define BTN_BACK_PIN        5    // strapping pin, safe: pull-up idles it HIGH

#endif // PINS_CONFIG_H
