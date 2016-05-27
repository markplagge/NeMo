//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_GLOBALS_H
#define NEMO_GLOBALS_H
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <limits.h>
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

//#define IABS(a) (((a) < 0) ? (-a) : (a))

/** Faster version  of IABS (no branching) but needs types. @todo this
 method will be faster on BGQ, but need to make sure that it works properly */
 weight_type IABS(weight_type in){
    //int_fast64_t const mask = in >> sizeof(int_fast64_t) * CHAR_BIT - 1;
    #ifdef HAVE_SIGN_EXTENDING_BITSHIFT
    int const mask = v >> sizeof(int) * CHAR_BIT - 1;
    #else
    int const mask = -((unsigned)in >> sizeof(int) * CHAR_BIT - 1);
    #endif
    return (in ^ mask) - mask;
}
//32bit X86 Assembler IABS:
int iIABS(int vals){

        int result;
        asm ("movl  %[valI], %%eax;"
                "cdq;"
                "xor %%edx, %%eax;"
                "sub %%edx, %%eax;"
                "movl %%eax, %[resI];"
        : [resI] "=r" (result)
        : [valI] "r" (vals)
        : "cc","%eax", "%ebx");
        return result;


}
/* Global Variables */
extern size_type  LPS_PER_PE;
extern size_type  SIM_SIZE;
extern size_type  LP_PER_KP;

extern bool IS_RAND_NETWORK;
extern size_type CORES_IN_SIM;
extern size_type NEURONS_IN_CORE;
extern size_type AXONS_IN_CORE;
extern size_type SIM_SIZE;
extern size_type CORE_SIZE;
extern size_type SYNAPSES_IN_CORE;

extern bool BULK_MODE;
extern bool DEBUG_MODE;
extern bool SAVE_MEMBRANE_POTS ;
extern bool SAVE_SPIKE_EVTS ;
extern bool SAVE_NEURON_OUTS;

extern bool PHAS_VAL;
extern bool TONIC_SPK_VAL;
extern bool TONIC_BURST_VAL;
extern bool PHASIC_BURST_VAL;
extern bool VALIDATION;





/* Global Timing Variables */
extern tw_stime littleTick;
extern tw_stime CLOCK_RANDOM_ADJ;

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

