#ifndef BIOSORPTION_STATE_MACHINE_H
#define BIOSORPTION_STATE_MACHINE_H

#include <Arduino.h>
#include "CONFIG.h"  // VOLUME_SANTOL, VOLUME_SAMPALOK, STIRRING_TIME,
                    // FILTER_END_FLOW_ML_MIN

// State of one treatment line
typedef enum {
  STATE_IDLE,
  STATE_FILLING,
  STATE_STIRRING,
  STATE_FILTERING,
  STATE_COMPLETE
} BiosorptionState;

// Current state of each line, modified by system_run()
extern BiosorptionState state_santol;
extern BiosorptionState state_sampalok;

void system_init();
void system_run();
void start_santol_cycle();
void start_sampalok_cycle();
void stop_cycle();

#endif // BIOSORPTION_STATE_MACHINE_H
