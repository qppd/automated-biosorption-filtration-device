#include "BIOSORPTION_STATE_MACHINE.h"
#include "RELAY_MODULE.h"
#include "FLOW_SENSOR_MODULE.h"
#include "LCD_MODULE.h"
#include "BUTTON_MODULE.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

// Current state of each treatment line (public, for status inspection)
BiosorptionState state_santol = STATE_IDLE;
BiosorptionState state_sampalok = STATE_IDLE;

// Flow sensor instances, defined in BiosorptionFiltration.ino
extern FlowSensorData flow_sensor_santol;
extern FlowSensorData flow_sensor_sampalok;

// One treatment line. Both lines run the same code with their own sensor,
// relays and constants.
typedef struct {
  const char* tag;            // short label shown on the LCD ("SA" / "SP")
  float fill_target_ml;       // volume that ends the fill phase
  unsigned long phase_start;  // millis() when the current phase began
  RelayIndex pump;
  RelayIndex motor;
  RelayIndex valve;
  FlowSensorData* sensor;     // on the fill path only
} Line;

static Line line_santol = {
  "SA", VOLUME_SANTOL, 0,
  RELAY_PUMP_SANTOL, RELAY_MOTOR_SANTOL, RELAY_VALVE_SANTOL,
  &flow_sensor_santol
};

static Line line_sampalok = {
  "SP", VOLUME_SAMPALOK, 0,
  RELAY_PUMP_SAMPALOK, RELAY_MOTOR_SAMPALOK, RELAY_VALVE_SAMPALOK,
  &flow_sensor_sampalok
};

// Switch every load of one line off.
static void line_all_off(const Line& line) {
  relay_off(line.pump);
  relay_off(line.motor);
  relay_off(line.valve);
}

void system_init() {
  state_santol = STATE_IDLE;
  state_sampalok = STATE_IDLE;
  line_santol.phase_start = 0;
  line_sampalok.phase_start = 0;

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

  lcd_write_row(0, "Biosorption");
  lcd_write_row(1, "Press START");
}

// ---------------------------------------------------------------------------
// Automatic phases. DISPENSING is deliberately not automatic: there is no
// flow sensor on the valve path, so the operator ends it with STOP.
// ---------------------------------------------------------------------------
static void run_line(Line& line, BiosorptionState& state) {
  unsigned long elapsed = millis() - line.phase_start;

  switch (state) {
    case STATE_IDLE:
    case STATE_COMPLETE:
    case STATE_FAULT:
      break;

    case STATE_FILLING:
      if (flow_sensor_get_total_volume_ml(line.sensor) >= line.fill_target_ml) {
        relay_off(line.pump);
        relay_on(line.motor);
        state = STATE_STIRRING;
        line.phase_start = millis();
      } else if (elapsed >= FILL_TIMEOUT_MS) {
        // Pump dry, sensor dead or reservoir empty: stop instead of running on.
        line_all_off(line);
        state = STATE_FAULT;
        Serial.print(line.tag);
        Serial.println(": fill timeout");
      }
      break;

    case STATE_STIRRING:
      if (elapsed >= STIRRING_TIME) {
        relay_off(line.motor);
        relay_on(line.valve);
        state = STATE_DISPENSING;
        line.phase_start = millis();
        Serial.print(line.tag);
        Serial.println(": dispensing");
      }
      break;

    case STATE_DISPENSING:
      // Nothing to detect: the valve stays open until STOP is pressed.
      break;
  }
}

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------
static void render_line(const Line& line, BiosorptionState state,
                        char* buf, size_t len) {
  switch (state) {
    case STATE_IDLE:
      snprintf(buf, len, "%s:IDLE", line.tag);
      break;

    case STATE_FILLING:
      // Pump state plus the volume the flow sensor has counted so far.
      snprintf(buf, len, "%s:P ON %4.1fmL", line.tag,
               flow_sensor_get_total_volume_ml(line.sensor));
      break;

    case STATE_STIRRING: {
      // Stirrer state plus the remaining contact time (mm:ss).
      unsigned long elapsed = millis() - line.phase_start;
      unsigned long left = (elapsed < STIRRING_TIME) ? (STIRRING_TIME - elapsed) : 0;
      snprintf(buf, len, "%s:M ON %02lu:%02lu", line.tag,
               left / 60000UL, (left / 1000UL) % 60UL);
      break;
    }

    case STATE_DISPENSING:
      snprintf(buf, len, "%s:VLV OPEN", line.tag);
      break;

    case STATE_COMPLETE:
      snprintf(buf, len, "%s:DONE", line.tag);
      break;

    case STATE_FAULT:
      snprintf(buf, len, "%s:TIMEOUT", line.tag);
      break;
  }
}

// The 16x2 display has two rows, so shared states get a combined screen and
// mixed states get one row per line.
static void update_display() {
  char top[LCD_COLS + 1];
  char bottom[LCD_COLS + 1];

  bool s_idle = (state_santol == STATE_IDLE);
  bool p_idle = (state_sampalok == STATE_IDLE);
  bool s_disp = (state_santol == STATE_DISPENSING);
  bool p_disp = (state_sampalok == STATE_DISPENSING);
  bool s_done = (state_santol == STATE_COMPLETE);
  bool p_done = (state_sampalok == STATE_COMPLETE);
  bool running = (state_santol == STATE_FILLING || state_santol == STATE_STIRRING ||
                  state_sampalok == STATE_FILLING || state_sampalok == STATE_STIRRING);

  if (s_idle && p_idle) {
    snprintf(top, sizeof top, "Biosorption");
    snprintf(bottom, sizeof bottom, "Press START");

  } else if ((s_disp || p_disp) && !running) {
    // At least one valve is open and nothing is filling or stirring. There is
    // no outlet flow sensor, so the operator ends the flow by hand.
    if (s_disp && p_disp) {
      snprintf(top, sizeof top, "SA:OPEN SP:OPEN");
    } else if (s_disp) {
      snprintf(top, sizeof top, "SA:OPEN");
    } else {
      snprintf(top, sizeof top, "SP:OPEN");
    }
    snprintf(bottom, sizeof bottom, "Press STOP");

  } else if (s_done && p_done) {
    snprintf(top, sizeof top, "SA:DONE SP:DONE");
    snprintf(bottom, sizeof bottom, "Press START");

  } else {
    render_line(line_santol, state_santol, top, sizeof top);
    render_line(line_sampalok, state_sampalok, bottom, sizeof bottom);
  }

  lcd_write_row(0, top);
  lcd_write_row(1, bottom);
}

// ---------------------------------------------------------------------------
// Operator input
// ---------------------------------------------------------------------------
static void handle_start() {
  bool santol_ready = (state_santol == STATE_IDLE || state_santol == STATE_COMPLETE);
  bool sampalok_ready = (state_sampalok == STATE_IDLE || state_sampalok == STATE_COMPLETE);
  if (!santol_ready || !sampalok_ready) return;

  start_santol_cycle();
  start_sampalok_cycle();
}

static void handle_stop() {
  bool running = (state_santol == STATE_FILLING || state_santol == STATE_STIRRING ||
                  state_sampalok == STATE_FILLING || state_sampalok == STATE_STIRRING);
  bool dispensing = (state_santol == STATE_DISPENSING || state_sampalok == STATE_DISPENSING);

  if (!running && dispensing) {
    // STOP was pressed while the "Press STOP" prompt was up: the product is
    // fully dispensed. Close the valves and keep the batch as complete.
    relay_all_off();
    if (state_santol == STATE_DISPENSING) state_santol = STATE_COMPLETE;
    if (state_sampalok == STATE_DISPENSING) state_sampalok = STATE_COMPLETE;
    // Clear any fault on the other line so the next START is accepted.
    if (state_santol == STATE_FAULT) state_santol = STATE_IDLE;
    if (state_sampalok == STATE_FAULT) state_sampalok = STATE_IDLE;
    Serial.println("Dispensing complete");
    return;
  }

  stop_cycle();
}

void system_run() {
  flow_sensor_update(&flow_sensor_santol);
  flow_sensor_update(&flow_sensor_sampalok);

  // STOP is handled before the state machines so it acts in the same pass.
  if (button_pressed(BTN_STOP)) {
    handle_stop();
  }

  run_line(line_santol, state_santol);
  run_line(line_sampalok, state_sampalok);

  if (button_pressed(BTN_START)) {
    handle_start();
  }

  update_display();
}

// ---------------------------------------------------------------------------
// Cycle control
// ---------------------------------------------------------------------------
static void start_line(Line& line, BiosorptionState& state) {
  flow_sensor_reset_total_volume(line.sensor);

  relay_on(line.pump);
  relay_off(line.motor);
  relay_off(line.valve);

  state = STATE_FILLING;
  line.phase_start = millis();
}

void start_santol_cycle() {
  if (state_santol != STATE_IDLE && state_santol != STATE_COMPLETE) return;
  start_line(line_santol, state_santol);
  Serial.println("Santol: filling");
}

void start_sampalok_cycle() {
  if (state_sampalok != STATE_IDLE && state_sampalok != STATE_COMPLETE) return;
  start_line(line_sampalok, state_sampalok);
  Serial.println("Sampalok: filling");
}

void stop_cycle() {
  // Hard stop / reset: every relay off, both lines back to idle, faults
  // cleared, so a new cycle can be started straight away.
  relay_all_off();
  state_santol = STATE_IDLE;
  state_sampalok = STATE_IDLE;
  Serial.println("Force stop");
}
