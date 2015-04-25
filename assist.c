/** Assist.C
*  Contains helper functions, used in the model.
*  Functions here should be too generic to include
*  in neuron/synapse
* Contains all of the global defs as well.
*/

/**
* getcoreID - returns the current core, based on
* the lpID.
* CURRENTLY RETURNS 1 until multi-core sims are
* developed.
* @param lpID
* @return
*/
#include "assist.h"

long getTotalSynapses() {
   return SYNAPSES_IN_CORE * CORES_IN_SIM;
}
long getTotalNeurons() {
	return NEURONS_IN_CORE * CORES_IN_SIM;
}
tw_stime getNextEventTime(tw_lp *lp){

    return tw_rand_unif(lp->rng) / 10;
}