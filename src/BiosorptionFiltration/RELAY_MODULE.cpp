#include "RELAY_MODULE.h"

void relay_init() {
  for (uint8_t i = 0; i < RELAY_COUNT; ++i) {
    pinMode(relayPins[i], OUTPUT);
    // Initialize to OFF state (based on active low/high)
    relay_off(static_cast<RelayIndex>(i));
  }
}

void relay_on(RelayIndex relay) {
  if (relay >= RELAY_COUNT) return;
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(relayPins[relay], LOW);  // Active low: LOW turns on
  } else {
    digitalWrite(relayPins[relay], HIGH); // Active high: HIGH turns on
  }
}

void relay_off(RelayIndex relay) {
  if (relay >= RELAY_COUNT) return;
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(relayPins[relay], HIGH); // Active low: HIGH turns off
  } else {
    digitalWrite(relayPins[relay], LOW);  // Active high: LOW turns off
  }
}

void relay_all_off() {
  for (uint8_t i = 0; i < RELAY_COUNT; ++i) {
    relay_off(static_cast<RelayIndex>(i));
  }
}
