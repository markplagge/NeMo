//
// Created by Mark Plagge on 5/26/16.
//

#ifndef NEMO_NEMO_MAIN_H
#define NEMO_NEMO_MAIN_H

#define EXTERN
#include "globals.h"
#undef EXTERN
#include <stdio.h>
#include "nemo_config.h"
#include "./neuro/axon.h"
#include "./neuro/synapse.h"
#include "./neuro/tn_neuron.h"
#include "ross.h"
#include "./tests/nemo_tests.h"
#include "./IO/IOStack.h"
#include "./IO/input.h" 
#include "./IO/spike_reader.h"

#endif //NEMO_NEMO_MAIN_H
