#include "neuron.h"
#include "synapse.h"
#include "axon.h"
#include "ross.h"

/**
 * @brief      Sets up the neuron, axon, and synapse event handler function pointers
 *
 * @param[in]  neuronType   The neuron type
 * @param[in]  synapseType  The synapse type
 * @param[in]  axonType     The axon type
 */
void initHandlers(int neuronType, int synapseType, int axonType);


void neuron_init(neuronState *s, tw_bf *CV, messageData *M. tw_lp *lp);
void neuron_event(neuronState *s, tw_bf *CV, messageData *M, tw_lp *lp);
void neuron_reverse(neuronState *, tw_bf *, messageData *, tw_lp *);
void neuron_final(neuronState *s, tw_lp *lp);


void synapse_init(synapseState *s, tw_lp *lp);
void synapse_event(synapseState *s, tw_bf *, messageData *M, tw_lp *lp);
void synapse_reverse(synapseState *, tw_bf *, messageData *M, tw_lp *);
void synapse_final(synapseState *s, tw_lp *lp);

void axon_init(axonState *s, tw_lp *lp);
void axon_event(axonState *s, tw_bf *, messageData *M, tw_lp *lp);
void axon_reverse(axonState *, tw_bf *, messageData *M, tw_lp *);
void axon_final(axonState *s, tw_lp *lp);
