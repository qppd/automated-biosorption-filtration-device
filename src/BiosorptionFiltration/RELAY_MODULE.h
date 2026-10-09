#ifndef RELAY_MODULE_H
#define RELAY_MODULE_H

#include <Arduino.h>
#include "PINS_CONFIG.h"  // All relay GPIOs defined centrally here
#include "CONFIG.h"        // Relay behaviour (RELAY_ACTIVE_LOW)

// Relay pin definitions: RELAY_1_PIN .. RELAY_8_PIN live in PINS_CONFIG.h
// (ESP32 38-pin WROOM safe pins - no boot-strapping, UART0, flash or I2C pins)

// Number of relays
#define RELAY_COUNT           8

// Relay active state (RELAY_ACTIVE_LOW) is centralized in CONFIG.h

// Relay index enumeration
typedef enum {
  RELAY_PUMP_SANTOL      = 0,
  RELAY_PUMP_SAMPALOK    = 1,
  RELAY_MOTOR_SANTOL     = 2,
  RELAY_MOTOR_SAMPALOK   = 3,
  RELAY_VALVE_SANTOL     = 4,
  RELAY_VALVE_SAMPALOK   = 5,
  RELAY_RESERVED1        = 6,
  RELAY_RESERVED2        = 7
} RelayIndex;

// Relay pin array for easy iteration
const uint8_t relayPins[RELAY_COUNT] = {
  RELAY_1_PIN,
  RELAY_2_PIN,
  RELAY_3_PIN,
  RELAY_4_PIN,
  RELAY_5_PIN,
  RELAY_6_PIN,
  RELAY_7_PIN,
  RELAY_8_PIN
};

// Function prototypes
void relay_init();
void relay_on(RelayIndex relay);
void relay_off(RelayIndex relay);
void relay_all_off();

#endif // RELAY_MODULE_H