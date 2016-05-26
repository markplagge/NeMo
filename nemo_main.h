//
// Created by Mark Plagge on 5/26/16.
//

#ifndef NEMO_NEMO_MAIN_H
#define NEMO_NEMO_MAIN_H

#include "globals.h"
#include "nemo_config.h"
#include "ross.h"

size_type  LPS_PER_PE;
size_type  SIM_SIZE;
size_type  LP_PER_KP;
unsigned int RAND_WT_PROB;
bool DEBUG_MODE;
bool BASIC_SOP;
bool TW_DELTA;
bool BULK_MODE;
bool PHAS_VAL;
bool TONIC_SPK_VAL;
bool TONIC_BURST_VAL;
bool PHASIC_BURST_VAL;
bool DEPOLAR_VAL ;
bool SAVE_MEMBRANE_POTS ;
bool SAVE_SPIKE_EVTS ;
bool SAVE_NEURON_OUTS;

bool validation;
thresh_type THRESHOLD_MAX;
thresh_type NEG_THRESHOLD_MAX;

tw_stime littleTick;
tw_stime CLOCK_RANDOM_ADJ;

#endif //NEMO_NEMO_MAIN_H
