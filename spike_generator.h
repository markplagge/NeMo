/**
*  \file spike_generator.h
*	spike_generate defines a LP state and functions that generate output at a
*	tuneable rate. Tool that generates spikes. Currently, a pseudo random input function is included.
*  TNT_MAIN
*  Created by Mark Plagge on 4/14/15. @author Mark Plagge
*
**/

#ifndef __ROSS_TOP__spike_generator__
#define __ROSS_TOP__spike_generator__

#include <stdio.h>
#include "assist.h"
/** Enum that selects the type of random number generation */
typedef enum RandomSelect {
	UNF,
	EXP,
	BINOM,
	GEOM,
	SELECT
}randomSelect;


/** SpikeGenerator Function Pointer. Chooses the spike method function. */
typedef bool (*spikeGenDel)(void *spikeGen, tw_lp *lp);

bool randomGen(void *gen_state, tw_lp *lp);
bool uniformGen(void *gen_state, tw_lp *lp);
bool geometricGen(void *gen_state, tw_lp *lp);
bool expGen(void *gen_state, tw_lp *lp);
bool binGen(void *gen_state, tw_lp *lp);

bool selectedGen(void *spikeGen, tw_lp *lp);


	/** Struct that genreates spikes randomly. */
typedef struct RandomSpikes {
	float randomRate; /**< If the random value is over this, spike.*/
	randomSelect randMethod; /**< Selected random generator. */
	float rndFctVal; /**<For functions that need a second parameter (eg, binomial etc.), this is the second parameter. */
}randomSpikes;

typedef struct SelectedSpikes {
	int * outputMesh; /**< Array that represents the output levels per tick.*/
	int outputMeshLengh; /**< Size of the output mesh. */
}selectedSpikes;

/** Struct that manages the spike generator. Generally, there should be one of these
 *	per simulation! */
typedef struct SpikeGenerator {
    int outbound; /**< Represents how many conenctions the input system is attached to in spike_generator_model#outbound. */
	tw_lpid *connectedSynapses; /**< An array of synapses that this is RandomSpikes is attached to. */
	spikeGenDel spikeGen;

		//selectedSpikes  selSpikes;
		//randomSpikes  rndSpikes;
	int * outputMesh; /**< Array that represents the output levels per tick.*/
	int outputMeshLengh; /**< Size of the output mesh. */
	float randomRate; /**< If the random value is over this, spike.*/
	randomSelect randMethod; /**< Selected random generator. */
	float rndFctVal; /**<For functions that need a second parameter (eg, binomial etc.), this is the second parameter. */
    int currentOut; /**< currentOut is the currently selected output synapse. Used in sequential mode */

    /**
     * @brief genMode selected spike generation mode.
     */
	randomSelect genMode;
    randomSelect selectMode;

}spikeGenState;



#endif /* defined(__ROSS_TOP__spike_generator__) */

