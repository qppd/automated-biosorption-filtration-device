#ifndef BIOSORPTION_STATE_MACHINE_H
#define BIOSORPTION_STATE_MACHINE_H

#include <Arduino.h>
#include "CONFIG.h"  // VOLUME_SANTOL, VOLUME_SAMPALOK, STIRRING_TIME,
                    // FILL_TIMEOUT_MS

// State of one treatment line
typedef enum {
  STATE_IDLE,
  STATE_FILLING,
  STATE_STIRRING,
  STATE_DISPENSING,  // valve open; ended by the operator with STOP
  STATE_COMPLETE,
  STATE_FAULT        // fill timeout expired; cleared with the STOP button
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
