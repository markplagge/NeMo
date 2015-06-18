//
//  assist.h
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/17/15.
//
//

#ifndef __ROSS_TOP__assist__
#define __ROSS_TOP__assist__

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include "ross.h"
/***Type definitions for the nuron simulation */

#define _idT uint_fast32_t //ID type - local id type for bit shifts and ID cases.
#define _voltT int_fast32_t ///Voltage data type
#define _statT int_fast64_t ///Counter data type for stats
/** _regIDT is a "regional id" type. This variable type is for storing
 *	coreIDs and localIDs. It must be half the bit size of tw_lpid. @todo: add
 *	macro to adjust the bit width. */
#define _regIDT uint32_t

/* Global Macros */

/** IABS is an integer absolute value function */
#define IABS(a) (((a) < 0) ? (-a) : (a))
	/** RZER is a floor function - values below zero round up to zero */
#define RZER(a) (((a) < 0) ? (0) : (a))

/* simulation structs. TODO: Maybe move these into main? */
/** evtType is a message/event identifier flag */
enum evtType {
	AXON_OUT, ///Message originates from an axon
	AXON_HEARTBEAT, ///Axon heartbeat message - big clock synchronization.
	SYNAPSE_OUT, ///Message originates from a synapse
	NEURON_OUT, ///Message originates from a neuron, and is going to an axion.
	NEURON_HEARTBEAT, ///Neuron heartbeat messages - for big clock syncronization.
	GEN_HEARTBEAT ///Signal generator messages -- used to simulate input for benchmarking.
};
/* Message structures */
/** Main message struct */
typedef struct {
	enum evtType eventType;
	unsigned int rndCallCount;
}Msg_Data;

/* ***** Global variable defs */
extern int NEURONS_IN_CORE;
extern int CORES_IN_SIM;
extern int AXONS_IN_CORE;
extern int SYNAPSES_IN_CORE;
extern unsigned int GEN_ON;
extern bool GEN_RND;
extern unsigned int RND_MODE;
extern unsigned int GEN_PROB;
extern unsigned int GEN_FCT;
extern unsigned int GEN_OUTBOUND;
extern unsigned int GEN_SEL_MODE;
extern unsigned int SP_DBG;

extern int THRESHOLD_MAX;
extern int THRESHOLD_MIN;
extern int SYNAPSE_WEIGHT_MAX;
extern int SYNAPSE_WEIGHT_MIN;
;
extern int CORE_SIZE;

	//helper functions:
tw_stime getNextEventTime(tw_lp *lp);


#endif /* defined(__ROSS_TOP__assist__) */
