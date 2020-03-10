// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_SYNAPSE_H
#define NEMO_SYNAPSE_H

#include "../globals.h"
#include "../mapping.h"
#include "../IO/IOStack.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct SynapseState {
  stat_type msgSent;
  tw_stime lastBigTick;
  id_type myCore;
  tw_bf neuronBF[NEURONS_IN_CORE];
  unsigned long randCount[NEURONS_IN_CORE];
  bool connectionGrid[NEURONS_IN_CORE][NEURONS_IN_CORE];
} synapse_state;
void synapse_init(synapse_state *s, tw_lp *lp);
void synapse_event(synapse_state *s, tw_bf *bf, messageData *M, tw_lp *lp);
void synapse_reverse(synapse_state *s, tw_bf *bf, messageData *M, tw_lp *lp);
void synapse_final(synapse_state *s, tw_lp *lp);
void synapse_pre_run(synapse_state *s, tw_lp *lp);
#ifdef __cplusplus
};
#endif
#endif //NEMO_SYNAPSE_H
