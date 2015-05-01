/** Assist.C
*  Contains helper functions, used in the model.
*  Functions here should be too generic to include
*  in neuron/synapse
* Contains all of the global defs as well.
*/


#include "assist.h"
/**
 *  getTotalSynapses - simple function that gets the total synapses globally
 *
 *  @return The number of synapses in in the sim
 */
long getTotalSynapses() {
   return SYNAPSES_IN_CORE * (CORES_PER_PE * g_tw_npe);
}
/**
 *  getTotalNeurons - simple function that gets the total synapses globally
 *
 *  @return The number of neurons in the system.
 */
long getTotalNeurons() {
	return NEURONS_IN_CORE * CORES_PER_PE * g_tw_npe;
}
/**
 *  Gets the next event time, based on a random function. Moved here to allow for
 *	easier abstraciton, and random function replacement.
 *
 *
 *  @param lp Reference to the current LP so that the function can see the RNG
 *
 *  @return a tw_stime value, such that \f$ 0 < t < 1 \f$. A delta for the next
 *  time slice.
 */
tw_stime getNextEventTime(tw_lp *lp){

    return tw_rand_unif(lp->rng) / 10;
}