#ifndef BUTTON_MODULE_H
#define BUTTON_MODULE_H

#include <Arduino.h>
#include "PINS_CONFIG.h"  // button GPIOs
#include "CONFIG.h"        // BUTTON_ACTIVE_LOW, DEBOUNCE_DELAY

// GPIO assignment: PINS_CONFIG.h. Buttons must be ordinary GPIOs; the
// input-only pins 34-39 have no internal pull-ups and cannot be used
// with INPUT_PULLUP.

#define BTN_COUNT 4

// Button identifiers. IDs are 1-based; the buttons[] array is 0-based
// (index = id - BTN_UP).
typedef enum {
  BTN_NONE = 0,
  BTN_UP,
  BTN_DOWN,
  BTN_SELECT,
  BTN_BACK
} ButtonID;

// Per-button input state, maintained by button_update()
typedef struct {
  uint8_t pin;
  bool lastState;              // last debounced state
  bool currentState;           // current debounced state
  bool lastReading;            // previous raw sample
  unsigned long lastDebounceTime;  // time of the last raw change
} ButtonData;

extern ButtonData buttons[BTN_COUNT];

void button_init();
void button_update();
bool button_pressed(ButtonID id);
bool button_released(ButtonID id);
bool button_state(ButtonID id);
ButtonID button_get_pressed();
ButtonID button_get_released();

#endif // BUTTON_MODULE_H
