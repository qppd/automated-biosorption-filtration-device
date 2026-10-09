#include "BIOSORPTION_STATE_MACHINE.h"
#include "RELAY_MODULE.h"
#include "FLOW_SENSOR_MODULE.h"
#include "LCD_MODULE.h"
#include "BUTTON_MODULE.h"
#include <Arduino.h>

// Current state of each treatment line
BiosorptionState state_santol = STATE_IDLE;
BiosorptionState state_sampalok = STATE_IDLE;

// Time (ms) at which each stirrer was started
static unsigned long stir_start_time_santol = 0;
static unsigned long stir_start_time_sampalok = 0;

// Flow sensor instances, defined in BiosorptionFiltration.ino
extern FlowSensorData flow_sensor_santol;
extern FlowSensorData flow_sensor_sampalok;

void system_init() {
  state_santol = STATE_IDLE;
  state_sampalok = STATE_IDLE;

  // Flow sensors: pins come from PINS_CONFIG.h; every ESP32 GPIO is
  // interrupt-capable
  flow_sensor_init(&flow_sensor_santol, FLOW_SENSOR_SANTOL_PIN);
  flow_sensor_init(&flow_sensor_sampalok, FLOW_SENSOR_SAMPALOK_PIN);

  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_SANTOL_PIN), []() {
    flow_sensor_isr(&flow_sensor_santol);
  }, RISING);

  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_SAMPALOK_PIN), []() {
    flow_sensor_isr(&flow_sensor_sampalok);
  }, RISING);

  flow_sensor_reset_total_volume(&flow_sensor_santol);
  flow_sensor_reset_total_volume(&flow_sensor_sampalok);

  relay_all_off();

  setLCDText("System Initialized", 0, 0);
  setLCDText("Ready", 0, 1);
}

void system_run() {
  flow_sensor_update(&flow_sensor_santol);
  flow_sensor_update(&flow_sensor_sampalok);

  // Santol line
  switch (state_santol) {
    case STATE_IDLE:
      break;

    case STATE_FILLING:
      if (flow_sensor_get_total_volume_ml(&flow_sensor_santol) >= VOLUME_SANTOL) {
        relay_off(RELAY_PUMP_SANTOL);
        state_santol = STATE_STIRRING;
        stir_start_time_santol = millis();
        relay_on(RELAY_MOTOR_SANTOL);
        setLCDText("Santol: Stirring", 0, 0);
      }
      break;

    case STATE_STIRRING:
      if (millis() - stir_start_time_santol >= STIRRING_TIME) {
        relay_off(RELAY_MOTOR_SANTOL);
        relay_on(RELAY_VALVE_SANTOL);
        state_santol = STATE_FILTERING;
        setLCDText("Santol: Filtering", 0, 0);
      }
      break;

    case STATE_FILTERING:
      if (flow_sensor_get_flow_rate_ml_min(&flow_sensor_santol) < FILTER_END_FLOW_ML_MIN) {
        relay_off(RELAY_VALVE_SANTOL);
        state_santol = STATE_COMPLETE;
        setLCDText("Santol: Complete", 0, 0);
      }
      break;

    case STATE_COMPLETE:
      break;
  }

  // Sampalok line
  switch (state_sampalok) {
    case STATE_IDLE:
      break;

    case STATE_FILLING:
      if (flow_sensor_get_total_volume_ml(&flow_sensor_sampalok) >= VOLUME_SAMPALOK) {
        relay_off(RELAY_PUMP_SAMPALOK);
        state_sampalok = STATE_STIRRING;
        stir_start_time_sampalok = millis();
        relay_on(RELAY_MOTOR_SAMPALOK);
        setLCDText("Sampalok: Stirring", 0, 1);
      }
      break;

    case STATE_STIRRING:
      if (millis() - stir_start_time_sampalok >= STIRRING_TIME) {
        relay_off(RELAY_MOTOR_SAMPALOK);
        relay_on(RELAY_VALVE_SAMPALOK);
        state_sampalok = STATE_FILTERING;
        setLCDText("Sampalok: Filtering", 0, 1);
      }
      break;

    case STATE_FILTERING:
      if (flow_sensor_get_flow_rate_ml_min(&flow_sensor_sampalok) < FILTER_END_FLOW_ML_MIN) {
        relay_off(RELAY_VALVE_SAMPALOK);
        state_sampalok = STATE_COMPLETE;
        setLCDText("Sampalok: Complete", 0, 1);
      }
      break;

    case STATE_COMPLETE:
      break;
  }

  // Operator input: Select starts both lines when both are idle
  ButtonID pressed = button_get_pressed();
  if (pressed == BTN_SELECT) {
    if (state_santol == STATE_IDLE && state_sampalok == STATE_IDLE) {
      start_santol_cycle();
      start_sampalok_cycle();
    }
  }
}

void start_santol_cycle() {
  if (state_santol != STATE_IDLE) return;

  flow_sensor_reset_total_volume(&flow_sensor_santol);

  relay_on(RELAY_PUMP_SANTOL);
  relay_off(RELAY_VALVE_SANTOL);
  relay_off(RELAY_MOTOR_SANTOL);

  state_santol = STATE_FILLING;
  setLCDText("Santol: Filling...", 0, 0);
}

void start_sampalok_cycle() {
  if (state_sampalok != STATE_IDLE) return;

  flow_sensor_reset_total_volume(&flow_sensor_sampalok);

  relay_on(RELAY_PUMP_SAMPALOK);
  relay_off(RELAY_VALVE_SAMPALOK);
  relay_off(RELAY_MOTOR_SAMPALOK);

  state_sampalok = STATE_FILLING;
  setLCDText("Sampalok: Filling...", 0, 1);
}

void stop_cycle() {
  // Emergency stop: all relays off, both lines back to idle
  relay_all_off();
  state_santol = STATE_IDLE;
  state_sampalok = STATE_IDLE;
  setLCDText("Stopped", 0, 0);
  setLCDText("Stopped", 0, 1);
}
