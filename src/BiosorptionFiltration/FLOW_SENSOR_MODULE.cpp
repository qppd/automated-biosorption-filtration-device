#include "FLOW_SENSOR_MODULE.h"
#include <Arduino.h>

void flow_sensor_init(FlowSensorData* sensor, uint8_t pin) {
  sensor->pin = pin;
  sensor->pulse_count = 0;
  sensor->flow_rate_ml_min = 0.0;
  sensor->flow_rate_l_hr = 0.0;
  sensor->total_volume_ml = 0.0;
  sensor->total_volume_l = 0.0;
  sensor->last_measurement_time = 0;
  sensor->sensor_active = false;
  pinMode(pin, INPUT_PULLUP);

  // The interrupt is attached by the caller (see BIOSORPTION_STATE_MACHINE.cpp)
  // with attachInterrupt(digitalPinToInterrupt(pin), ..., RISING). Every ESP32
  // GPIO can generate interrupts.
}

// Called from the interrupt service routine
void flow_sensor_isr(FlowSensorData* sensor) {
  // Increment written out: "++" on a volatile-qualified type is deprecated.
  sensor->pulse_count = sensor->pulse_count + 1;
}

void flow_sensor_update(FlowSensorData* sensor) {
  unsigned long now = millis();
  unsigned long window = now - sensor->last_measurement_time;
  if (window < FLOW_MEASURE_INTERVAL) {
    return;
  }

  // Take and clear the counter in one critical section. Reading and then
  // clearing separately would lose a pulse that arrives in between.
  noInterrupts();
  uint32_t pulses = sensor->pulse_count;
  sensor->pulse_count = 0;
  interrupts();

  // Volume follows directly from the pulse count (FLOW_CALIBRATION_FACTOR
  // pulses per litre), so it does not depend on how long the loop actually
  // took between measurements.
  float volumeAddedML = (pulses * 1000.0) / FLOW_CALIBRATION_FACTOR;
  sensor->total_volume_ml += volumeAddedML;
  sensor->total_volume_l = sensor->total_volume_ml / 1000.0;

  // Flow rate uses the real elapsed window, not the nominal interval.
  float mlPerSecond = volumeAddedML * 1000.0 / window;
  sensor->flow_rate_ml_min = mlPerSecond * 60.0;
  sensor->flow_rate_l_hr = sensor->flow_rate_ml_min * 0.06;  // 1 mL/min = 0.06 L/hr

  sensor->last_measurement_time = now;
  sensor->sensor_active = true;
}

float flow_sensor_get_flow_rate_ml_min(FlowSensorData* sensor) {
  return sensor->flow_rate_ml_min;
}

float flow_sensor_get_flow_rate_l_hr(FlowSensorData* sensor) {
  return sensor->flow_rate_l_hr;
}

float flow_sensor_get_total_volume_ml(FlowSensorData* sensor) {
  return sensor->total_volume_ml;
}

float flow_sensor_get_total_volume_l(FlowSensorData* sensor) {
  return sensor->total_volume_l;
}

void flow_sensor_reset_total_volume(FlowSensorData* sensor) {
  sensor->total_volume_ml = 0.0;
  sensor->total_volume_l = 0.0;
  // Drop pulses counted before the cycle started so they are not attributed
  // to the new fill. Called while the pump is stopped (see start_line()).
  noInterrupts();
  sensor->pulse_count = 0;
  interrupts();
}

bool flow_sensor_is_active(FlowSensorData* sensor) {
  return sensor->sensor_active;
}
