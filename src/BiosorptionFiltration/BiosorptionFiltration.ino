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
  // Serial is used for fault and phase logging only; the LCD stays the
  // primary operator interface.
  Serial.begin(115200);

  // Order matters: display, then outputs, then inputs. system_init()
  // initialises the flow sensors, attaches their interrupts and writes the
  // "Press START" screen.
  lcd_init();
  relay_init();
  button_init();
  system_init();
}

void loop() {
  button_update();
  system_run();
  delay(SYSTEM_LOOP_DELAY_MS);
}
