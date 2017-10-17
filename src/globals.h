//
// Created by Mark Plagge on 5/25/16.
//
//
/** @file globals.h */



#ifndef __NEMO_GLOBALS_H__
#define __NEMO_GLOBALS_H__

#define BGQ 1
#define NET_IO_DEBUG 1

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

#define SAVE_NEURON_STATS 1

/**@}*/
/**
 * Neuron file read buffer size - for reading CSV files.
 */
#define NEURON_BUFFER_SZ  32
#define MAX_NEURON_PARAMS  65536

#define TXT_HEADER "****************************************************************************\n"
#define TH printf( TXT_HEADER );
#define STT( str,p ) printf("* \t" str "\n" , p );

/** @defgroup types Typedef Vars 
 * Typedefs to ensure proper types for the neuron parameters/mapping calculations
 */
/**@{  */

typedef uint_fast64_t id_type; //!< id type is used for local mapping functions - there should be $n$ of them depending on CORE_SIZE
typedef int32_t volt_type; //!< volt_type stores voltage values for membrane potential calculations
typedef int64_t weight_type;//!< seperate type for synaptic weights.
typedef uint32_t thresh_type;//!< Type for weights internal to the neurons.
typedef uint16_t random_type;//!< Type for random values in neuron models.

typedef uint64_t size_type; //!< size_type holds sizes of the sim - core size, neurons per core, etc.

typedef uint64_t stat_type;
/**@}*/

/** @defgroup cmacros Global Dynamic Typing casting macros for file IO */
/** @{ */
#define ATOX(x) _Generic((x), \
double: atof,\
long: atol,\
int: atoi,\
float: atof



/** @defgroup gmacros Global Macros and Related Functions
 *Global Macros */
/**@{ */

//Switched from C11 variables to generic print function:
void debugMsg(char * m, char * d);
#define print printf
#define pm(x) printf("%s",x);
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

/** Macro for use within globals. 
 Assumes that there is a tw_lp pointer called lp in the function it is used.
 */
#define JITTER (tw_rand_unif(lp->rng) / 10000)

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
enum neuronTypeVals {
	NA = 0,
    TN = 1
};

/**
 * @brief      test result flag.
 */
enum mapTestResults {
  INVALID_AXON = 1 << 1, //!< Axon was not properly defined.
  INVALID_SYNAPSE = 1 << 2, //!< Synapse was not properly defined.
  INVALID_NEURON = 1 << 3 //!< Neuron was not properly defined.
};

//typedef enum NeuronTypes {
//    TrueNorth = 0
//} neuronTypes;

typedef enum LayerTypes{
    NON_LAYER =          0,
    GRID_LAYER =         1 << 1,
    CONVOLUTIONAL_LAYER =1 << 2,
	OUTPUT_UNQ =    1 << 3,
	OUTPUT_RND = 1 << 4

}layerTypes;
/**@} */

//Message Structure (Used Globally so placed here)
typedef struct Ms{
    enum evtType eventType;
    unsigned long rndCallCount;
    id_type localID; //!< Sender's local (within a core) id - used for weight lookups.
    unsigned long long isRemote;
	//long double remoteRcvTime;
	union{
		unsigned long synapseCounter;
		struct{
			volt_type neuronVoltage;
			tw_stime neuronLastActiveTime;
			tw_stime neuronLastLeakTime;
			random_type neuronDrawnRandom;
		};
	};
	
	
	// union{
        id_type axonID; //!< Axon ID for neuron value lookups.
        bool * neuronConn;
	//};
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

	//tw_stime msgCreationTime;
#endif
}messageData;
 /**@}*/

/** \defgroup IOStructs Input/Output structs @{ */
/** Structs for managing neuron reads */
typedef struct CsvNeuron{
    int fld_num;
    int req_core_id;
    int req_local_id;
    int foundNeuron;
    char rawDatM[MAX_NEURON_PARAMS][NEURON_BUFFER_SZ];
//	char *rawDatP[NEURON_BUFFER_SZ];
//    char **rawDat;

}csvNeuron;
/**
 * SpikeElem / spikeElem is a struct that holds the raw spike data
 * from a CSV file. The data is stored in a simclist list. One spike
 * is stored in each spikeElem.
 */
typedef struct SpikeElem{
    long scheduledTime;
    long destCore;
    long destAxon;
}spikeElem;

/**
 * NeuronMembraneRecord stores an active neuron's membrane potential.
 * This is used to save and store membrane potential / neuron activity
 * during a simulation run. Used by output.c
 */
typedef struct NeuronMembraneRecord{
	tw_stime tickTime;
	volt_type membranePot;
	id_type neuronCore;
	id_type neuronID;
}neuronMembraneRecord;







/** @} */

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

//EXT size_type AXONS_IN_CORE;
EXT size_type SIM_SIZE;
EXT size_type CORE_SIZE;
EXT size_type SYNAPSES_IN_CORE;

EXT bool BULK_MODE;
EXT bool DEBUG_MODE;
EXT bool SAVE_MEMBRANE_POTS ;
EXT bool SAVE_SPIKE_EVTS ; //!< Toggles saving spike events
EXT bool SAVE_NETWORK_STRUCTURE;

//EXT bool MPI_SAVE;
EXT bool BINARY_OUTPUT;

EXT bool PHAS_VAL;
//EXT bool TONIC_SPK_VAL;
EXT bool TONIC_BURST_VAL;
EXT bool PHASIC_BURST_VAL;
EXT bool VALIDATION;

EXT bool FILE_OUT;
EXT bool FILE_IN;

/** value for the size of a processor. Defaults to 4096 cores per proc */
EXT unsigned int CORES_IN_CHIP;
EXT unsigned int NUM_CHIPS_IN_SIM;
EXT unsigned int CHIPS_PER_RANK;


EXT char * NEURON_FIRE_R_FN	;


/** @defgroup ctime Compute Time Parameters
 * Variables that change DUMPI compute time / send time @{
 */
EXT long double COMPUTE_TIME  ;
EXT long double SEND_TIME_MIN ;
EXT long double SEND_TIME_MAX ;
EXT unsigned int DO_DUMPI ;

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

/** @defgroup satnet Saturation network flags / settings @{ */
EXT unsigned int SAT_NET_PERCENT;
EXT unsigned int SAT_NET_COREMODE;
EXT unsigned int SAT_NET_THRESH;
EXT unsigned int SAT_NET_LEAK;
EXT unsigned int SAT_NET_STOC;
EXT unsigned int IS_SAT_NET;

/** @} @defgroup laynet Basic Layer Network Settings @{ */
//typedef struct LParams {
//    unsigned long numLayersInSim;
//    layerTypes LAYER_NET_MODE;
//    unsigned int layerSizes[4096];
//};
EXT unsigned int NUM_LAYERS_IN_SIM;
EXT layerTypes LAYER_NET_MODE;
EXT unsigned int LAYER_SIZES[4096];
EXT unsigned int CORES_PER_LAYER;
EXT unsigned int CHIPS_PER_LAYER;
EXT unsigned int GRID_ENABLE;
EXT unsigned int GRID_MODE;
EXT unsigned int RND_GRID;
EXT unsigned int RND_UNIQ;
EXT unsigned int UNEVEN_LAYERS;
EXT char * LAYER_LAYOUT;
/**@} @defgroup iocfg File buffer settings
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


/** @defgroup fileNames File Names
 * Vars that manage file names for IO.
 * These variables are declared/init in input.c and
 * output.c @{*/

/** @} */
