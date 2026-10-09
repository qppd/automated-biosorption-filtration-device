#ifndef FLOW_SENSOR_MODULE_H
#define FLOW_SENSOR_MODULE_H

#include <Arduino.h>
#include "PINS_CONFIG.h"  // Flow sensor GPIOs defined centrally here
#include "CONFIG.h"        // Calibration + intervals

// Flow sensor pin definitions: FLOW_SENSOR_SANTOL_PIN / FLOW_SENSOR_SAMPALOK_PIN
// live in PINS_CONFIG.h (GPIO 26 / 27 - interrupt-capable, no boot or UART pins)

// Calibration (FLOW_CALIBRATION_FACTOR) and measurement interval
// (FLOW_MEASURE_INTERVAL) are centralized in CONFIG.h

// Flow sensor structure
typedef struct {
  uint8_t pin;                     // Pin number for this sensor
  volatile uint32_t pulse_count;   // Pulse counter (updated in ISR)
  float flow_rate_ml_min;          // Current flow rate in mL/min
  float flow_rate_l_hr;            // Current flow rate in L/hr
  float total_volume_ml;           // Total volume passed in mL
  float total_volume_l;            // Total volume passed in L
  uint32_t last_measurement_time;  // Last measurement timestamp
  bool sensor_active;              // Sensor status flag
} FlowSensorData;

// Function prototypes
void flow_sensor_init(FlowSensorData* sensor, uint8_t pin);
void flow_sensor_isr(FlowSensorData* sensor);        // Interrupt Service Routine
void flow_sensor_update(FlowSensorData* sensor);     // Call periodically in loop()
float flow_sensor_get_flow_rate_ml_min(FlowSensorData* sensor);
float flow_sensor_get_flow_rate_l_hr(FlowSensorData* sensor);
float flow_sensor_get_total_volume_ml(FlowSensorData* sensor);
float flow_sensor_get_total_volume_l(FlowSensorData* sensor);
void flow_sensor_reset_total_volume(FlowSensorData* sensor);
bool flow_sensor_is_active(FlowSensorData* sensor);

#endif // FLOW_SENSOR_MODULE_H