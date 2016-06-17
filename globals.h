//
// Created by Mark Plagge on 5/25/16.
//
//
/** @file globals.h */



#ifndef __NEMO_GLOBALS_H__
#define __NEMO_GLOBALS_H__




#include <inttypes.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include <nemo_config.h>
#include "ross.h"


/** @defgroup types Typedef Vars 
 * Typedefs to ensure proper types for the neuron parameters/mapping calculations
 */
/**@{  */

typedef int_fast16_t id_type; //!< id type is used for local mapping functions - there should be $n$ of them depending on CORE_SIZE
typedef int32_t volt_type; //!< volt_type stores voltage values for membrane potential calculations
typedef int64_t weight_type;//!< seperate type for synaptic weights.
typedef uint32_t thresh_type;//!< Type for weights internal to the neurons.
typedef uint16_t random_type;//!< Type for random values in neuron models.

typedef uint64_t size_type; //!< size_type holds sizes of the sim - core size, neurons per core, etc.

typedef uint64_t stat_type;
/**@}*/

/* @defgroup gmacros Global Macros and Related Functions
 *Global Macros */
/**@{ */


#define IABS(a) (((a) < 0) ? (-a) : (a)) //!< Typeless integer absolute value function

#define SGN(x) ((x > 0) - (x < 0)) //!< Signum function

#define DT(x) !(x) //!<Kronecker Delta function.

#define BINCOMP(s,p) IABS((s)) >= (p) //!< binary comparison for conditional stochastic evaluation
//32bit X86 Assembler IABS:
int iIABS(int vals);

weight_type iiABS(weight_type in);
/** @} */

/** @defgroup timeFuncts Time Functions
  * Functions that manage big tick and little ticks
  */
  /** @{ */

/**
 *  Gets the next event time, based on a random function. Moved here to allow for
 *  easier abstraction, and random function replacement.
 *
 *
 *  @param lp Reference to the current LP so that the function can see the RNG
 *
 *  @return a tw_stime value, such that \f$ 0 < t < 1 \f$. A delta for the next
 *  time slice.
 */
tw_stime getNextEventTime(tw_lp *lp);
/**
 *  @brief  Given a tw_stime, returns the current big tick.
 *
 *  @param now current time
 *
 *  @return the current big tick time.
 */
tw_stime getCurrentBigTick(tw_stime now);

/**
 *  @brief  Given a tw_stime, returns the next big-tick that will happen
 *
 *  @param lp the lp asking for the next big tick.
 *  @param neuronID Currently unused. Reserved for future fine-grained neuron tick management.
 *
 *  @return Next big tick time.
 */
tw_stime getNextBigTick(tw_lp *lp, tw_lpid neuronID);

/**@}*/

/** @defgroup global_structs_enums Global Structs and Enums
  * Global structs and enums, including event types, lp types, and the message structure
  */
  /** @{ */

/** evtType is a message/event identifier flag */
enum evtType {
    AXON_OUT, //!< Message originates from an axon
    AXON_HEARTBEAT, //!< Axon heartbeat message - big clock synchronization.
    SYNAPSE_OUT, //!< Message originates from a synapse
    NEURON_OUT, //!< Message originates from a neuron, and is going to an axion.
    NEURON_HEARTBEAT, //!< Neuron heartbeat messages - for big clock syncronization.
    NEURON_SETUP, //!< Message that contains a neuron's setup information for the synapse - connectivity info
    GEN_HEARTBEAT //!< Signal generator messages -- used to simulate input for benchmarking.
};
enum lpTypeVals {
    AXON = 0,
    SYNAPSE = 1,
    NEURON = 2
};

/**
 * @brief      test result flag.
 */
enum mapTestResults {
  INVALID_AXON = 0x01, //!< Axon was not properly defined.
  INVALID_SYNAPSE = 0x02, //!< Synapse was not properly defined.
  INVALID_NEURON = 0x03 //!< Neuron was not properly defined.
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
    random_type neuronDrawnRandom;
    union{
        id_type axonID; //!< Axon ID for neuron value lookups.
        bool * neuronConn;
    };


    //neuron state saving extra params:



   

}messageData;
 /**@}*/

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

EXT bool FILE_OUT;
EXT bool FILE_IN;



/* Global Timing Variables */
/**
 * little tick rate - controls little tick timing
 */
EXT tw_stime littleTick;
/**
 * clock random value adjuster.
 */
EXT tw_stime CLOCK_RANDOM_ADJ;
/** @} */
#endif