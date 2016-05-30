//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_SYNAPSE_H
#define NEMO_SYNAPSE_H
#include "../globals.h"
typedef struct SynapseState {
	tw_lpid destSynapse;
	tw_lpid destNeuron;
	//@todo remove this cruft
	id_t mySynapseNum;
	stat_type msgSent;

	
}synapseState;

#endif //NEMO_SYNAPSE_H
