/* 
 * File:   synapse.h
 * Author: plaggm
 *
 * Created on February 26, 2015, 4:41 PM
 */

#ifndef SYNAPSE_H
#define    SYNAPSE_H
#include "../spike_generator.h"
#ifdef    __cplusplus
extern "C" {
#endif

//Synapses have a dest. core and an ID number.

typedef struct SynapseState {
   regid_t synID;
   regid_t coreID;
   gid_t *dests;
   //unsigned int coreDest;
	spikeGenState *spikeGen;
} synapseState;


#ifdef    __cplusplus
}
#endif

#endif	/* SYNAPSE_H */

