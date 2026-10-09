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
  sensor->pulse_count++;
}

void flow_sensor_update(FlowSensorData* sensor) {
  if ((millis() - sensor->last_measurement_time) < FLOW_MEASURE_INTERVAL) {
    return;
  }

  uint32_t pulses = sensor->pulse_count;
  sensor->pulse_count = 0;

  // FLOW_CALIBRATION_FACTOR pulses per liter, counted over the last
  // FLOW_MEASURE_INTERVAL milliseconds
  float pulsesPerSecond = (pulses * 1000.0) / FLOW_MEASURE_INTERVAL;
  float mlPerSecond = pulsesPerSecond * (1000.0 / FLOW_CALIBRATION_FACTOR);
  sensor->flow_rate_ml_min = mlPerSecond * 60.0;
  sensor->flow_rate_l_hr = sensor->flow_rate_ml_min * 0.06;  // 1 mL/min = 0.06 L/hr

  float volumeAddedML = mlPerSecond * (FLOW_MEASURE_INTERVAL / 1000.0);
  sensor->total_volume_ml += volumeAddedML;
  sensor->total_volume_l = sensor->total_volume_ml / 1000.0;
  sensor->last_measurement_time = millis();
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
}

bool flow_sensor_is_active(FlowSensorData* sensor) {
  return sensor->sensor_active;
}
