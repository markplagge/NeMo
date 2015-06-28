//
//  mapping.h
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#ifndef __ROSS_TOP__mapping__
#define __ROSS_TOP__mapping__

#include <stdio.h>
#include "ross.h"
#include "assist.h"


#define lc 0
#define co 1
#define ISIDE(x) ((uint16_t*)&x)[0]
#define JSIDE(x) ((uint16_t*)&x)[1]
#define CORE(x) ((uint32_t*)&x)[1]
#define LOCAL(x) ((uint32_t*)&x)[0]

#define _gridIDT uint16_t



extern int NEURONS_IN_CORE ;
/**
 *  Number of synapses per core.
 */
extern int SYNAPSES_IN_CORE;
extern int CORE_SIZE;
extern unsigned int SIM_SIZE;
extern int NUM_VP_X;
extern int NUM_VP_Y;
extern int VP_PER_PROC;
extern unsigned int LPS_PER_PE;
extern unsigned int LP_PER_KP;
extern unsigned int nkp_per_pe;
	//extern tw_lptype model_lps[];
	//extern unsigned int SIM_SIZE;
extern tw_lptype model_lps[];

extern mapTypes tnMapping;
tw_lpid *myGIDs;
tw_lpid *gePEMap;

tw_lpid lpTypeMapper(tw_lpid gid);


/**
 * @brief CORE_LP_OFFSET - Manages the offset. Calculated based on the size of a core,
 * and the CPE vale, the number of PEs required to simulate a single core.
 * For example, if core size is 128, and CPE is 2, then each PE will get 64 LPs.
 * TODO: Add a synapse and neuron balancing function.
 */
int CORE_LP_OFFSET;
extern int CPE;

/**
 *  @brief  Custom Mapping - given a LP GID return a PE
 *
 *  @param gid <#gid description#>
 *
 *  @return <#return value description#>
 */
tw_peid getPEFromGID(tw_lpid gid);


_idT peToCoreMap(tw_peid pe);


/**
 *  @brief  Setup mapping per PE
 */
void mappingSetup();

tw_lpid localToGlobal(tw_lpid local);
/**
 *  @brief  Given a global ID, return an LP.
 *
 *  @param gid <#gid description#>
 *
 *  @return <#return value description#>
 */
tw_lp * globalToLP(tw_lpid gid);

tw_lpid globalToLocalID(tw_lpid gid);

/**
 * @brief getCoreFromGID returns the coreID when given an LP's globalID.
 * @param gid
 * @return the core that the lp at \a gid is running on.
 */
long getCoreFromGID(tw_lpid gid);

/**
 * @brief getCoreFromPE returns the coreID assigned to a PE.
 * @param gid
 * @return
 */
long getCoreFromPE(tw_peid gid);

/**
 * @brief getCoreLocalFromGID calculates the localID (neuron model) of the LP based on the global ID.
 * This is not the ROSS local, this is the neuron model local ID.
 * @param gid
 * @return
 */
long getCoreLocalFromGID(tw_lpid gid);

tw_lpid globalID(_idT core, uint16_t i, uint16_t j);
tw_lpid getGlobalFromID(_idT core, _idT local);
	//neurons are stored at at i = 256, j 0-265
tw_lpid getNeuronGlobal(_idT core, uint16_t neuron);
	//axons are stored at i = 0, j = 0-256
tw_lpid getAxonGlobal(_idT core, uint16_t axon);

tw_lpid getSynapseGlobal(_idT core, _idT synapse);
/**
 *  @brief  Get synapse from synapse should be called from a synapse - it returns the next logical synapse GID, or aborts if the synpase is at the end of the grid.
 *
 *  @param synapse current synaspe
 *
 *  @return returns a synapse gid.
 */
tw_lpid getSynapseFromSynapse(tw_lpid synapse);
/**
 *  @brief  get neuron from synapse gives the GID of the neuron a synapse should send a message to.
 *

 *  @param synapse the synapse
 *
 *  @return a neuron's gid
 */
tw_lpid getNeuronFromSynapse(tw_lpid synapse);

tw_lpid getSynapseFromAxon(tw_lpid axon);

void nlMap();
void scatterMap();
/**
 *  @brief  Based on the PCS grid mapping idea, this takes the 3-D structure of the TN architecture,  where neurons, synapses, and axons are X & Y, and cores are Z, and maps the proper LP types out.
 */
void tn_cube_mapping();
/**
 *  @brief  Type mapping based on standard linear map.
 *
 *  @param gid current GID
 *
 *  @return int for the type array
 */
tw_lpid tn_linear_map(tw_lpid gid);

tw_peid lGidToPE(tw_lpid gid);

tw_lpid lGetSynFromAxon(tw_lpid axeGID);
tw_lpid lGetNextSynFromSyn(tw_lpid synGID);
tw_lpid lGetNeuronFromSyn(tw_lpid synGID);
tw_lpid lGetAxonFromNeu(_idT core, _idT axeNum);
tw_lpid lCoreOffset(tw_lpid gid);
tw_lpid lGetSynNumLocal(tw_lpid gid);
tw_lpid lGetAxeNumLocal(tw_lpid gid);
tw_lpid lgetNeuNumLocal(tw_lpid gid);
_gridIDT iSizeOffset();
_gridIDT jSizeOffset();

/**
 *  @brief  returns the decimal sum of the i,j, and core values from a gid.
 *
 *  @param gid <#gid description#>
 *
 *  @return <#return value description#>
 */
_idT combVal(tw_lpid gid);

bool dontSkip(tw_lpid gid);

#endif /* defined(__ROSS_TOP__mapping__) */
