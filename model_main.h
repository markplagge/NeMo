//
//  model_main.h
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/17/15.
//
//

#ifndef __ROSS_TOP__model_main__
#define __ROSS_TOP__model_main__

#include <stdio.h>
#include "assist.h"
#include "ross.h"
#include "models/axon.h"
#include "models/neuron.h"
#include "models/synapse.h"
#include <stdbool.h>

	// Variable holders for command lne params & external variables */
/**
 *  Number of neurons per core.
 */
int NEURONS_IN_CORE = 128;

/** number of synapses per core. Calculated value, needs to be neurons * axons */
int SYNSAPES_IN_CORE;
/* Calculated number of cores in simulation */
int CORES_IN_SIM;

/** Number of axions per core. Generally is set to 1-1 with neurons in core */
int AXONS_IN_CORE = 128;

/** Simulation tuning variables */

/** noise generator values */
unsigned int GEN_ON = 1;
bool GEN_RND = 1;
unsigned int RND_MODE = 0;
unsigned int GEN_PROB = 50;
unsigned int GEN_FCT = 5;
unsigned int GEN_OUTBOUND = 0;
unsigned int GEN_SEL_MODE = 0;
unsigned int SP_DBG = 0;


/**  Determines the maximum and minimum thresholds for a neuron to fire.
 */
int THRESHOLD_MAX = 100;
/**
 *  Minimum threshold. @see THRESHOLD_MAX
 */
int THRESHOLD_MIN = 30;
/**
 *	Each neuron is connected to the synapses (inputs) within the core it is running in.
 *	These parameters adjust the input weight given to each synapse. */
int SYNAPSE_WEIGHT_MAX = 10;
/** Minimum synapse weight. @see SYNAPSE_WEIGHT_MAX */
int SYNAPSE_WEIGHT_MIN = 1;
tw_stime PER_SYNAPSE_DET_P = .50;

	//Neuron Tuning variables:

	//Simulation Variables
/**CORE_SIZE is equal to the number of axions * number of aneurons + num neurons + num axions */
int CORE_SIZE;

/* **** Model Main Function */

void createLPs();
void pre_run();
void neuron_init(neuronState *s, tw_lp *lp);
void setSynapseWeight(neuronState *s, tw_lp *lp, int synapseID);
void neuron_event(neuronState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp);
void neuron_reverse(neuronState *, tw_bf *, Msg_Data *, tw_lp *);
void neuron_final(neuronState *s, tw_lp *lp);

void synapse_init(synapseState *s, tw_lp *lp);
void synapse_event(synapseState *s, tw_bf *, Msg_Data *M, tw_lp *lp);
void synapse_reverse(synapseState *, tw_bf *, Msg_Data *M, tw_lp *);
void synapse_final(synapseState *s, tw_lp *lp);

void axon_init(axonState *s, tw_lp *lp);
void axon_event(axonState *s, tw_bf *, Msg_Data *M, tw_lp *lp);
void axon_reverse(axonState *, tw_bf *, Msg_Data *M, tw_lp *);
void axon_final(axonState *s, tw_lp *lp);

void mapping(tw_lp gid);


#endif /* defined(__ROSS_TOP__model_main__) */
