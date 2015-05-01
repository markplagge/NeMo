/*
 * File:   assist.h
 * Author: Mark Plagge -- plaggm
 *
 *  Contains helper functions, used in the model.
 *  Functions here should be too generic to include
 *  in neuron/synapse
 * Contains all of the global defs as well
 * When this project gets bigger, some serious rewrites will be needed.
 * As of now, these values are pretty arbitrary..


 * Created on February 28, 2015, 4:24 PM
 */
/** Parameter Defs:
 * Neurons in Core
 * Max Syapse Weight
 * Threshold Max
 * Parameters for tuning neurons.
 * */
// replaced with global vars:
/*
 #define NEURONS_IN_CORE 	4
 //#define SYNAPSES_IN_CORE	128
 #define SYNAPSES_IN_CORE	NEURONS_IN_CORE * 3 //our sim is 1:1 atm.
 #define CORES_PER_PE 	        1
 #define CORE_SIZE		NEURONS_IN_CORE + SYNAPSES_IN_CORE

 //maximum neuron threshold value:
 #define THRESHOLD_MAX 		50
 #define THRESHOLD_MIN 		1
 #define SYNAPSE_WEIGHT_MAX 	100
 // some params:
 #define DENDRITE_MIN 		1
 #define DENDRITE_MAX 		128
 #define DENDRITE_W_MIN 		1
 #define DENDRITE_W_MAX 		100
 */

#ifndef ASSIST_H
#define ASSIST_H

#include "ross.h"
#include <stdbool.h>
#include <sys/types.h>
#include <inttypes.h>
#ifdef __cplusplus
extern "C" {
#endif

extern int NEURONS_IN_CORE;
extern int SYNAPSES_IN_CORE;
extern int CORES_PER_PE;
extern int CORE_SIZE;
extern int THRESHOLD_MAX;
extern int THRESHOLD_MIN;
extern int SYNAPSE_WEIGHT_MAX;
extern int SYNAPSE_WEIGHT_MIN;
extern int DENDRITE_MIN;
extern int DENDRITE_MAX;
extern int DENDRITE_W_MIN;
extern int DENDRITE_W_MAX;
extern float CLOCK_SPEED;

extern bool isFile;
// TODO: Remove this and use integegrated ROSS settings.
extern int events_per_pe;  // settings for ROSS?
extern int exec_memory;
extern int clock_rate;  // might want to use this.
// TODO: Add a rollback calculation:
extern tw_stime lookahead;
extern tw_stime end_time;

// debug toggle:
extern int DEBUG_MODE;

// Variable Size defs (for tweakability)

#define _idType int_fast16_t  //  unsigned int
// neuron specific type redefs - for potentially integrating weird bit length
// variable sizes or what not:
#define _neVoltType uint_fast32_t
#define _neStatType int_fast32_t
/** stat_t is a type for storing statistics from the running sim. */
#define stat_t uint_fast64_t
/** regid_t is a "regional id" type. This variable type is for storing
 *	coreIDs and localIDs. It must be half the bit size of tw_lpid. @todo: add
 *	macro to adjust the bit width. */
#define regid_t uint32_t



// hard coded sim paramaters:

#define END 1000
#define GVT_INT 16
// some helpful macros:
#define IABS(a) (((a) < 0) ? (-a) : (a))
#define REMZ(a) (((a) < 0) ? (0) : (a))
#define TIME_CEIL (0)
/** LOC(a) -- bitwise local id getter from a tw_lpid. */
#define LOC(a) ((regid_t)a) & 0xFFFFFFFF
/** CORE(a) -- a bitwise core id getter from a tw_lpid */
#define CORE(a) ((regid_t)(((tw_lpid)(a) >> 32) & 0xFFFFFFFF))

long getTotalNeurons();
long getTotalSynapses();

/// Message information:
/** State items */

enum events { NEURON = 1, SYNAPSE = 2, INIT = 3, SPKGN = 4 };

// synapse message - message from a synapse to a neuron
typedef struct {
} SynapseMessage;
// neuron message - message from a neuron to a synapse (via virtual dendrite)
typedef struct {
} NeuronMessage;

// Message Data and informational content.
typedef struct {
  tw_lpid sender;
  _idType senderLocalID;
  _idType destCore;
  _idType destLocalID;
  _idType sourceCore;
  enum events type;
  /** This saves the old state of the neuron, before firing, so that roll back
   * functions will occur. */
  _neVoltType prevVoltage;

} Msg_Data;

// ts function for events - just so I can see what works best.
tw_stime getNextEventTime(tw_lp* lp);
#ifdef __cplusplus
}
#endif

#endif /* ASSIST_H */
