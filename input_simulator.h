//
//  input_simulator.h
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#ifndef __ROSS_TOP__input_simulator__
#define __ROSS_TOP__input_simulator__

#include <stdio.h>
#include "assist.h"
#include "ross.h"

typedef enum SelectedRandom {
	UNF,
	NORM,
	SELECT
}selectedRandom;
/** SpikeGenerator Function Pointer. Chooses the spike method function. */
typedef bool (*spikeGenDel)(void *spikeGen, tw_lp *lp);
bool uniformGen(void *gen_state, tw_lp *lp);
bool normGen(void *gen_state, tw_lp *lp);
bool selectedGen(void *spikeGen, tw_lp *lp);


/** Struct that genreates spikes randomly. */
typedef struct RandomSpikes {
	float randomRate; /**< If the random value is over this, spike.*/
	selectedRandom randMethod; /**< Selected random generator. */
	float rndFctVal; /**<For functions that need a second parameter (eg, binomial etc.), this is the second parameter. */
}randomSpikes;

typedef struct SelectedSpikes {
	int * outputMesh; /**< Array that represents the output levels per tick.*/
	int outputMeshLengh; /**< Size of the output mesh. */
}selectedSpikes;

/** Struct that manages the spike generator. Generally, there should be one of these
 *	per simulation! */
typedef struct InputSimulator {
	int outbound; /**< Represents how many conenctions the input system is attached to in spike_generator_model#outbound. */
	tw_lpid *connectedSynapses; /**< An array of synapses that this is RandomSpikes is attached to. */
	spikeGenDel spikeGen;
		//selectedSpikes  selSpikes;
		//randomSpikes  rndSpikes;
	int * outputMesh; /**< Array that represents the output levels per tick.*/
	int outputMeshLengh; /**< Size of the output mesh. */
	float randomRate; /**< If the random value is over this, spike.*/
	selectedRandom randMethod; /**< Selected random generator. */
	float rndFctVal; /**<For functions that need a second parameter (eg, binomial etc.), this is the second parameter. */
	int currentOut; /**< currentOut is the currently selected output synapse. Used in sequential mode */

}inputSimulatorState;
#endif /* defined(__ROSS_TOP__input_simulator__) */
