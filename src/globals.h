//
// Created by Mark Plagge on 5/25/16.
//
//
/** @file globals.h */



#ifndef __NEMO_GLOBALS_H__
#define __NEMO_GLOBALS_H__

#define BGQ 1


#include <inttypes.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include <nemo_config.h>
#include "ross.h"

/** @defgroup tempConfig Temporary configuration globals
 *	These global defines are stored here before I migrate them into either a run-time
 *	or compile-time option 
 * @{ */

#define SAVE_NEURON_STATS

/**@}*/

#define TXT_HEADER "****************************************************************************\n"


/** @defgroup types Typedef Vars 
 * Typedefs to ensure proper types for the neuron parameters/mapping calculations
 */
/**@{  */

typedef uint_fast16_t id_type; //!< id type is used for local mapping functions - there should be $n$ of them depending on CORE_SIZE
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

//Switched from C11 variables to generic print function:
void debugMsg(char * m, char * d);
#define print printf
//

/** TODO: Eventually replace this with generic macro and non-branching ABS code. */
#define IABS(a) (((a) < 0) ? (-a) : (a)) //!< Typeless integer absolute value function
/** TODO: See if there is a non-branching version of the signum function, maybe in MAth libs and use that. */
#define SGN(x) ((x > 0) - (x < 0)) //!< Signum function

#define DT(x) !(x) //!<Kronecker Delta function.

#define BINCOMP(s,p) IABS((s)) >= (p) //!< binary comparison for conditional stochastic evaluation
/** 32bit X86 Assembler IABS: */
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

tw_stime getNextSynapseHeartbeat(tw_lp *lp);
/** @defgroup global_structs_enums Global Structs and Enums
  * Global structs and enums, including event types, lp types, and the message structure
  */
  /** @{ */

/** evtType is a message/event identifier flag */
enum evtType {
    AXON_OUT, //!< Message originates from an axon
    AXON_HEARTBEAT, //!< Axon heartbeat message - big clock synchronization.
    SYNAPSE_OUT, //!< Message originates from a synapse
	SYNAPSE_HEARTBEAT, //!< Message is a synapse heartbeat message.
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
    unsigned long long isRemote;
	long double remoteRcvTime;
	union{
		unsigned long synapseCounter;
		struct{
			volt_type neuronVoltage;
			tw_stime neuronLastActiveTime;
			tw_stime neuronLastLeakTime;
			random_type neuronDrawnRandom;
		};
	};
	
	
    union{
        id_type axonID; //!< Axon ID for neuron value lookups.
        bool * neuronConn;
    };
    //message tracking values:
#ifdef SAVE_MSGS
    union {
        uint64_t uuid;
        struct {
            uint16_t idp1;
            uint16_t idp2;
            uint32_t idp3;
        };
    };
    tw_lpid originGID;
    char originComponent;

	tw_stime msgCreationTime;
#endif




   

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
EXT bool SAVE_SPIKE_EVTS ; //!< Toggles saving spike events
EXT bool SAVE_NEURON_OUTS;

EXT bool MPI_SAVE;
EXT bool BINARY_OUTPUT;

EXT bool PHAS_VAL;
EXT bool TONIC_SPK_VAL;
EXT bool TONIC_BURST_VAL;
EXT bool PHASIC_BURST_VAL;
EXT bool VALIDATION;

EXT bool FILE_OUT;
EXT bool FILE_IN;

/** value for the size of a processor. Defaults to 4096 cores per proc */
EXT unsigned int CORES_IN_CHIP;
EXT unsigned int NUM_CHIPS_IN_SIM;
EXT unsigned int CHIPS_PER_RANK;

/** @defgroup fileNames File Names
 * Vars that manage file names for IO  @{*/
EXT char * inputFileName;
EXT char * neuronFireFileName;

/** @} */


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


/** @defgroup iocfg File buffer settings
 * @{
 * */
/** POSIX Neuron Fire record Buffer Size */
EXT int N_FIRE_BUFF_SIZE;
//#define N_FIRE_BUFF_SIZE 32

/** POSIX Neuron Fire record line buffer size.
 * For text mode only, sets the length of strings stored in the neuron fire buffer*/
EXT int N_FIRE_LINE_SIZE;

//#define N_FIRE_LINE_SIZE 128
/** @} */
#endif
