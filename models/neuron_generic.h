//
// Created by plaggm on 2/3/16.
//
/** A generic neuron LP model. This defines the standard LP structure,
 * functions for accessing this struct, and generic neuron functions.
 * In NeMo, neurons have the following operational flow:
| Integration                               | Spike, Reset, Fire                     |
|-------------------------------------------|----------------------------------------|
| Receive Synapse Voltage                   | Receive Heartbeat Message              |
| 1. Modify Synapse Voltage                 | 1. Modify current membrane potential   |
| 2. Add Synapse Voltage to current Voltage | 2. Apply leak voltage(s) if applicable |
| 3. Send Heartbeat Message                 | 3. Check if spike condition is met     |
|                                           | 4. Spike                               |
|                                           | 5. Reset Voltage                       |
|                                           | 6. Post Reset Voltage Modification     |

 *
 * This file defines function stubs for each one of these operations.
 * This file also provides the neuron LP state structure in a generic form.
 * Before initialization, the LP state structure must be initialized to
 * the proper model's structure. */

#ifndef ROSS_TOP_NEURON_GENERIC_H
#define ROSS_TOP_NEURON_GENERIC_H
#include "../assist.h"
#include "../neuron_out_stats.h"
#include "ross.h"
#include <math.h>
#include <stdbool.h>

//Externs:
extern int NEURONS_IN_CORE;
extern unsigned int CORES_IN_SIM;
extern int AXONS_IN_CORE;
extern int SYNAPSES_IN_CORE;

//typedef void (*resetFunDel)(void *neuronState);

typedef struct GenericNeuronModel {

};

#endif //ROSS_TOP_NEURON_GENERIC_H
