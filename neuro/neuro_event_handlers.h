#ifndef __NEMO_NEURO_EVENT_HANDLERS_H__
#define __NEMO_NEURO_EVENT_HANDLERS_H__
#include "neuron.h"
#include "synapse.h"
#include "axon.h"
#include "ross.h"
//#include "../globals.h"
#include "../mapping.h"

/**
 * @brief      Sets up the neuron, axon, and synapse event handler function pointers
 *
 * @param[in]  neuronType   The neuron type
 * @param[in]  synapseType  The synapse type
 * @param[in]  axonType     The axon type
 */
void initHandlers(int neuronType, int synapseType, int axonType);

void neuron_init(neuronState *s, tw_lp *lp);
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

/**
 * \defgroup IBMFunctions IBM Function handlers
 * @{
 */
/**
 * @brief      creates an IBM Neuron
 *
 * @param      s     Neuron State
 * @param      lp    The pointer to a
 */
void IBM_neuron_init(neuronState *s, tw_lp *lp);

void IBM_neuron_event(neuronState *s, tw_bf *CV, messageData *M, tw_lp *lp);
void IBM_neuron_reverse(neuronState *, tw_bf *, messageData *, tw_lp *);
void IBM_neuron_final(neuronState *s, tw_lp *lp);


void IBM_synapse_init(synapseState *s, tw_lp *lp);
void IBM_synapse_event(synapseState *s, tw_bf *, messageData *M, tw_lp *lp);
void IBM_synapse_reverse(synapseState *, tw_bf *, messageData *M, tw_lp *);
void IBM_synapse_final(synapseState *s, tw_lp *lp);

void IBM_axon_init(axonState *s, tw_lp *lp);
void IBM_axon_event(axonState *s, tw_bf *, messageData *M, tw_lp *lp);
void IBM_axon_reverse(axonState *, tw_bf *, messageData *M, tw_lp *);
void IBM_axon_final(axonState *s, tw_lp *lp);


void IBM_create_simple_neuron(neuronState *s, tw_lp *lp);
/** Creates a neuron using standard spiking parameters. Reset voltage is  calculated
 here as VR * sigmaVR for model compatibility */
void  IBMinitNeuron(id_type coreID, id_type nID,
                 bool synapticConnectivity[],
                 short G_i[], short sigma[4], short S[4], bool b[4], bool epsilon,
                 short sigma_l, short lambda, bool c, uint32_t alpha,
                 uint32_t beta, short TM, short VR, short sigmaVR, short gamma, bool kappa, neuronState *n, int signalDelay, uint64_t destGlobalID, int destAxonID);
/** Creates a neuron using the encoded reset value method. Use this for more
 complete compatability with TrueNorth */
void IBMinitNeuronEncodedRV(id_type coreID, id_type nID,
                         bool synapticConnectivity[NEURONS_IN_CORE],
                         short G_i[NEURONS_IN_CORE], short sigma[4],
                         short S[4], bool b[4], bool epsilon,
                         short sigma_l, short lambda, bool c, uint32_t alpha,
                         uint32_t beta, short TM, short VR, short sigmaVR, short gamma,
                         bool kappa, neuronState *n, int signalDelay, uint64_t destGlobalID,int destAxonID);
/**
 *  @brief  handles incomming synapse messages. In this model, the neurons send messages to axons during "big tick" intervals.
 This is done through an event sent upon receipt of the first synapse message of the current big-tick.
 *
 *  @param st   current neuron state
 *  @param time time event was received
 *  @param m    event message data
 *  @param lp   lp.
 */
bool IBMneuronReceiveMessage(neuronState *st, messageData *M, tw_lp *lp, tw_bf *bf);
/**
 *  @brief  function that adds a synapse's value to the current neuron's membrane potential.
 *
 *  @param synapseID localID of the synapse sending the message.
 */
void IBMintegrate(id_type synapseID,neuronState *st, void *lp);


/**
 *  @brief  Checks to see if a neuron should fire. @todo check to see if this is needed, since it looks like just a simple if statement is in order.
 *
 *  @param st neuron state
 *
 *  @return true if the neuron is ready to fire.
 */
bool IBMneuronShouldFire(neuronState *st, void *lp);

/**
 * @brief New firing system using underflow/overflow and reset.
 * @return true if neuron is ready to fire. Membrane potential is set regardless.
 */
bool IBMfireFloorCelingReset(neuronState *ns, tw_lp *lp);


/**
 *  @brief  Function that runs after integration & firing, for reset function and threshold bounce calls.
 *
 *  @param st      state
 *  @param time    event time
 *  @param lp      lp
 *  @param didFire did the neuron fire during this big tick?
 */
void IBMneuronPostIntegrate(neuronState *st, tw_stime time, tw_lp *lp, bool didFire);
/**
 *  @brief  Neuron stochastic integration function - for use with stochastic leaks and synapse messages.
 *
 *  @param weight weight of selected leak or synapse
 *  @param st     the neuron state
 */
void IBMstochasticIntegrate(weight_type weight, neuronState *st);

/**
 * @brief      configures the IBM neuron destination
 *
 * @param[in]  signalDelay  The signal delay
 * @param[in]  globalID     The global id
 * @param      n            { parameter_description }
 */
void IBMsetNeuronDest(int signalDelay, uint64_t globalID, neuronState *n);

/**
 *  @brief NumericLeakCalc - uses formula from the TrueNorth paper to calculate leak.
 *  @details Will run $n$ times, where $n$ is the number of big-ticks that have occured since
 *  the last integrate. Handles stochastic and regular integrations.
 *
 *  @TODO: self-firing neurons will not properly send messages currently - if the leak is divergent, the flag needs to be set upon neuron init.
 *  @TODO: does not take into consideration thresholds. Positive thresholds would fall under self-firing neurons, but negative thresholds should be reset.
 *  @TODO: test leaking functions
 */
void IBMnumericLeakCalc(neuronState *st, tw_stime now);

void IBMfire(neuronState *st, void *lp);

void IBMsetNeuronDest(int signalDelay, uint64_t gid, neuronState *n);

void IBMneuronReverseState(neuronState *s, tw_bf *CV, messageData *m, tw_lp *lp);

void IBMsendHeartbeat(neuronState *st, tw_stime time, void *lp);
void IBMlinearLeak(neuronState *neuron, tw_stime now);

/** @} */



#endif