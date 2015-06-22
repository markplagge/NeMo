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


extern int NEURONS_IN_CORE ;
/**
 *  Number of synapses per core.
 */
extern int SYNAPSES_IN_CORE;

extern int CORE_SIZE;

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

/**
 *  @brief  Custom Mapping - given a LP GID return a PE
 *
 *  @param gid <#gid description#>
 *
 *  @return <#return value description#>
 */
tw_peid lpToPeMap(tw_peid gid);
/**
 *  @brief  Setup mapping per PE
 */
void mappingSetup();


/**
 *  @brief  Given a global ID, return a LP.
 *
 *  @param gid <#gid description#>
 *
 *  @return <#return value description#>
 */
tw_lp * globalToLocal(tw_lpid gid);

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

tw_lpid globalID(_idT core, _idT local);

tw_lpid getNeuronID(_idT core, _idT neuron);

tw_lpid getSynapse(_idT core, _idT i, _idT j);

tw_lpid getSyapseFromSynapse(_idT core, _idT synapse);
tw_lpid getNeuronFromSynapse(_idT core, _idT synapse);

#endif /* defined(__ROSS_TOP__mapping__) */
