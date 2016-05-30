//
// Created by Mark Plagge on 5/25/16.
//



#ifndef __NEMO_GLOBALS_H__
#define __NEMO_GLOBALS_H__



#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include "ross.h"



 
/** Typedefs to ensure proper types for the neuron parameters/mapping calculations */

typedef int_fast16_t id_type; //!< id type is used for local mapping functions - there should be $n$ of them depending on @CORE_SIZE
typedef int32_t volt_type; //!< volt_type stores voltage values for membrane potential calculations
typedef int64_t weight_type;//!< seperate type for synaptic weights.
typedef uint32_t thresh_type;//!< Type for weights internal to the neurons.

typedef uint64_t size_type; //!< size_type holds sizes of the sim - core size, neurons per core, etc.

typedef uint64_t stat_type;
/*Global Macros */
/** IABS is an integer absolute value function */

#define IABS(a) (((a) < 0) ? (-a) : (a))

#define SGN(x) ((x > 0) - (x < 0))


/** Faster version  of IABS (no branching) but needs types. @todo this
 method will be faster on BGQ, but need to make sure that it works properly */
//  weight_type IABS(weight_type in){
//     //int_fast64_t const mask = in >> sizeof(int_fast64_t) * CHAR_BIT - 1;
//     #ifdef HAVE_SIGN_EXTENDING_BITSHIFT
//     int const mask = v >> sizeof(int) * CHAR_BIT - 1;
//     #else
//     int const mask = -((unsigned)in >> sizeof(int) * CHAR_BIT - 1);
//     #endif
//     return (in ^ mask) - mask;
// }
//32bit X86 Assembler IABS:
int iIABS(int vals);

/** @} */
/** evtType is a message/event identifier flag */
enum evtType {
    AXON_OUT, //!< Message originates from an axon
    AXON_HEARTBEAT, //!< Axon heartbeat message - big clock synchronization.
    SYNAPSE_OUT, //!< Message originates from a synapse
    NEURON_OUT, //!< Message originates from a neuron, and is going to an axion.
    NEURON_HEARTBEAT, //!< Neuron heartbeat messages - for big clock syncronization.
    GEN_HEARTBEAT //!< Signal generator messages -- used to simulate input for benchmarking.
};
enum lpTypeVals {
    AXON = 0,
    SYNAPSE = 1,
    NEURON = 2
};

typedef enum NeuronTypes {
    TrueNorth = 0
} neuronTypes;

//Message Structure (Used Globally so placed here)
typedef struct Ms{
    enum evtType eventType;
    unsigned long rndCallCount;
    id_type localID; //!< Sender's local (within a core) id - used for weight lookups.
    volt_type neuronVoltage;
    tw_stime neuronLastActiveTime;
    tw_stime neuronLastLeakTime;
    //neuron state saving extra params:
    id_type axonID; //!< Axon ID for neuron value lookups.
   

}messageData;
#endif //NEMO_GLOBALS_H
#ifndef EXTERN
#define EXT extern
/**
 * \defgroup Globals Global Variables 
 * @{
 */
EXT size_type  LPS_PER_PE;
EXT size_type  SIM_SIZE;
EXT size_type  LP_PER_KP;

EXT bool IS_RAND_NETWORK;
EXT size_type CORES_IN_SIM;
EXT size_type NEURONS_IN_CORE;
EXT size_type AXONS_IN_CORE;
EXT size_type SIM_SIZE;
EXT size_type CORE_SIZE;
EXT size_type SYNAPSES_IN_CORE;

EXT bool BULK_MODE;
EXT bool DEBUG_MODE;
EXT bool SAVE_MEMBRANE_POTS ;
EXT bool SAVE_SPIKE_EVTS ;
EXT bool SAVE_NEURON_OUTS;

EXT bool PHAS_VAL;
EXT bool TONIC_SPK_VAL;
EXT bool TONIC_BURST_VAL;
EXT bool PHASIC_BURST_VAL;
EXT bool VALIDATION;





/* Global Timing Variables */
/**
 * little tick rate - controls little tick timing
 */
EXT tw_stime littleTick; 
/**
 * clock random value adjuster. 
 */
EXT tw_stime CLOCK_RANDOM_ADJ;

#endif