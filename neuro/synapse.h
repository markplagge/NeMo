//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_SYNAPSE_H
#define NEMO_SYNAPSE_H
#include "../globals.h"
#include "../mapping.h"



typedef struct SynapseState {
	stat_type msgSent;
	tw_stime lastBigTick;
	id_type myCore;	
}synapseState;
void synapse_init(synapseState *s, tw_lp *lp);
void synapse_event(synapseState *s, tw_bf *, messageData *M, tw_lp *lp);
void synapse_reverse(synapseState *, tw_bf *, messageData *M, tw_lp *lp);
void synapse_final(synapseState *s, tw_lp *lp);

#endif //NEMO_SYNAPSE_H
