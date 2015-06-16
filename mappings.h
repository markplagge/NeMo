//
// Created by mplagge on 5/1/15.
//

/**
 *  Mappings.h - header file containing all of the mappings for 
 *	the TNT_benchmark project. Per core, all synapses are mapped
 *	such that the first N lps are synapses, and the next M are
 *	neurons, as determined by SYNAPSES_IN_CORE and NEURONS_IN_CORE.
 */

#ifndef ROSS_TOP_MAPPINGS_H
#define ROSS_TOP_MAPPINGS_H
#include <stdio.h>
#include<assert.h>
#include<ross.h>
#include"assist.h"

extern int NEURONS_IN_CORE ;
/**
 *  Number of synapses per core.
 */
extern int SYNAPSES_IN_CORE;
/**
 *  Each PE can have one or more virtual cores running during the simulation. Default is 2.
 */
extern int CORES_PER_PE;


extern int nlp_per_kp;
extern int nkp_per_pe;


/**
 * @brief CORE_LP_OFFSET - Manages the offset. Calculated based on the size of a core,
 * and the CPE vale, the number of PEs required to simulate a single core.
 * For example, if core size is 128, and CPE is 2, then each PE will get 64 LPs.
 * TODO: Add a synapse and neuron balancing function.
 */
int CORE_LP_OFFSET;
extern int CPE;

/** LOC(a) -- bitwise local id getter from a tw_lpid. */
	//#define LOC(a) ((regid_t)a) & 0xFFFFFFFF
/** CORE(a) -- a bitwise core id getter from a tw_lpid */
	//#define CORE(a) ((regid_t)(((tw_lpid)(a) >> 32) & 0xFFFFFFFF))

	//mapping function -- based on linear
tw_peid mapping(tw_lpid gid);
/**
 *  Get the local IDs from a GID
 *
 *  @param global Global ID
 *  @param core   coreID
 *  @param local  localID (based on the whole core - not synapse or neuron ID.
 */
void getLocalIDs(tw_lpid global, regid_t* core, regid_t* local);

/**
 *  returns the gloval ID given a coreID and a localID (not syn or neuron ID)
 *
 *  @param core  The core
 *  @param local The core process ID
 *
 *  @return A global ID
 */
tw_lpid globalID(regid_t core, regid_t local) ;

/**
 *  Given a global id, return the synapse number of the item.
 *
 *  @param gid A global ID
 *
 *  @return the synapse number
 */
regid_t getSynapseID(tw_lpid gid);
/**
 *  Given a global ID, return the neuron number of the item
 *
 *  @param gid The global ID
 *
 *  @return the neuron number
 */
regid_t getNeuronID(tw_lpid gid);

/**
 *  given a global id, sets the core and local to the type local
 *  IDs - Neuron number or Synapse number.
 *
 *  @param global the global ID
 *  @param core   The core number
 *  @param local  The neuron or synapse number.
 */
void getTypeLocalIDs(tw_lpid global, regid_t* core, regid_t* local);

tw_lp* mapping_to_local(tw_lpid global);

tw_peid mapping(tw_lpid gid);
void initial_mapping(void) ;

void initMapVars(int nInCore,int sInCore,int cpe);

tw_lpid getRandomSynapse();
#endif //ROSS_TOP_MAPPINGS_H
