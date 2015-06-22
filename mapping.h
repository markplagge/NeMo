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




#endif /* defined(__ROSS_TOP__mapping__) */
