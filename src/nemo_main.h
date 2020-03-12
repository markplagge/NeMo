//
// Created by Mark Plagge on 5/26/16.
//

#ifndef NEMO_NEMO_MAIN_H
#define NEMO_NEMO_MAIN_H
#ifdef __cplusplus
extern "C" {
#endif
#define EXTERN$

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
#include "./IO/spike_reader.h"
#include "IO/spike_db_reader.h"

#ifdef __cplusplus
}
#endif
#endif //NEMO_NEMO_MAIN_H
