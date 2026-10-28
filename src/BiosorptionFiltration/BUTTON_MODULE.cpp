#include "BUTTON_MODULE.h"

ButtonData buttons[BTN_COUNT];

void button_init() {
  uint8_t pins[BTN_COUNT] = {BTN_START_PIN, BTN_STOP_PIN};
  for (uint8_t i = 0; i < BTN_COUNT; ++i) {
    buttons[i].pin = pins[i];
    buttons[i].lastState = BUTTON_ACTIVE_LOW ? HIGH : LOW;  // not pressed
    buttons[i].currentState = buttons[i].lastState;
    buttons[i].lastReading = buttons[i].lastState;
    buttons[i].lastDebounceTime = 0;
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
}

void button_update() {
  for (uint8_t i = 0; i < BTN_COUNT; ++i) {
    bool reading = digitalRead(buttons[i].pin);

    // Restart the debounce timer on every raw change; accept the reading
    // once it has been stable for DEBOUNCE_DELAY.
    if (reading != buttons[i].lastReading) {
      buttons[i].lastDebounceTime = millis();
    }

    if ((millis() - buttons[i].lastDebounceTime) > DEBOUNCE_DELAY) {
      if (reading != buttons[i].lastState) {
        buttons[i].lastState = reading;
        buttons[i].currentState = reading;
      }
    }

    buttons[i].lastReading = reading;
  }
}

static inline bool isPressed(const ButtonData& b) {
  return b.currentState == (BUTTON_ACTIVE_LOW ? LOW : HIGH);
}

static inline bool wasPressed(const ButtonData& b) {
  return b.lastState == (BUTTON_ACTIVE_LOW ? LOW : HIGH);
}

// Map a ButtonID (BTN_NONE = 0, BTN_START = 1, BTN_STOP = 2) to its
// buttons[] index (BTN_START = 0, BTN_STOP = 1). Invalid for BTN_NONE
// and for values outside the button range.
static inline bool valid_button_id(ButtonID id) {
  return id >= BTN_START && id <= BTN_STOP;
}

bool button_pressed(ButtonID id) {
  if (!valid_button_id(id)) return false;
  const ButtonData& b = buttons[id - BTN_START];
  return !wasPressed(b) && isPressed(b);
}

bool button_released(ButtonID id) {
  if (!valid_button_id(id)) return false;
  const ButtonData& b = buttons[id - BTN_START];
  return wasPressed(b) && !isPressed(b);
}

bool button_state(ButtonID id) {
  if (!valid_button_id(id)) return false;
  return isPressed(buttons[id - BTN_START]);
}

ButtonID button_get_pressed() {
  for (uint8_t i = 0; i < BTN_COUNT; ++i) {
    ButtonID id = static_cast<ButtonID>(BTN_START + i);
    if (button_pressed(id)) {
      return id;
    }
  }
  return BTN_NONE;
}

ButtonID button_get_released() {
  for (uint8_t i = 0; i < BTN_COUNT; ++i) {
    ButtonID id = static_cast<ButtonID>(BTN_START + i);
    if (button_released(id)) {
      return id;
    }
  }
  return BTN_NONE;
}
