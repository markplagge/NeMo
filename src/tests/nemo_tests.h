/** @defgroup nemo_tests Nemo Testing Functions
*	Defines nemo tests and related functionality.
* @{
	*/

#ifndef nemo_tests_h
#define nemo_tests_h
#include "../neuro/tn_neuron.h"
#include "../globals.h"
#include "../neuro/synapse.h"
#include "../neuro/axon.h"

#include "../mapping.h"
#include "ross.h"
#include <stdbool.h>
#include "testIO.h"

/**
 * @file nemo_tests.h
 */
/*@}*/





/**
 * \ingroup nemo_tests
 * @brief      tests the LPID mapping. Given neurons per core, and a core size, will iterate through
 * all of the values and add them to an array of tw_lpids. These should match the axons, synapses and
 * neurons.
 *
 * @param[in]  neuronsPerCore  The neurons per core
 * @param[in]  coreSize        The core size
 *
 * @return     an array of tw_lpid's
 * 
 */
tw_lpid *testCreateLPID(int neuronsPerCore, int totalLPs);

/**
 * \ingroup nemo_tests
 * @brief      Tests the create LP function. This is the function called by tw_create_lps. This
 * function uses the global defined parameters that are used in a full NeMo simulation.
 *
 * @return     Returns an array of tw_lps - perfect for checking that the create lp function
 * has created the right types of lps.
 */
int **testCreateLPs();




/**
 * \ingroup nemo_tests
 * @brief      runs the suite of synapse testing functions
 *
 * @param[in]  neuronsInCore  The neurons in each core
 * @param[in]  cores          The number of cores
 *
 * @return     	returns true if the tests pass
 */
bool runSynapseTests(int neuronsInCore, int cores);

/**
 * @brief      tests the synapse creation function as it would be called from ROSS
 *
 * @param      s     Synapse State - predef. 
 * @param      lp    The pointer to a new LP
 *
 * @return     returns the initalized synapse state.
 */
synapse_state *testCreateSynapse(synapse_state *s, tw_lp *lp);

id_type testSynapseOutput(id_type axonInput);

#endif
