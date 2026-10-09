// Automated Biosorption Filtration Device
// Entry point: initialises the modules, then runs the per-line state machines.

#include "LCD_MODULE.h"
#include "RELAY_MODULE.h"
#include "BUTTON_MODULE.h"
#include "FLOW_SENSOR_MODULE.h"
#include "BIOSORPTION_STATE_MACHINE.h"

// Flow sensor instances shared with the state machine
FlowSensorData flow_sensor_santol;
FlowSensorData flow_sensor_sampalok;

void setup() {
  // Order matters: display, then outputs, then inputs. system_init()
  // initialises the flow sensors and attaches their interrupts.
  lcd_init();
  relay_init();
  button_init();
  system_init();

  setLCDText("Biosorption System", 0, 0);
  setLCDText("Ready", 0, 1);
}

void loop() {
  button_update();
  system_run();
  delay(SYSTEM_LOOP_DELAY_MS);
}
