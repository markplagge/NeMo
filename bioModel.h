/**
 //
 //  bioModel.h
 //  ROSS_TOP
 //
 //  Created by Mark Plagge on 11/11/15.
 //
 //  @brief Creates neurons based on basic biological functions, based on iskavitch etc
 */

#ifndef bioModel_h
#define bioModel_h

#include <stdio.h>
#include "models/neuron.h"
#include "ross.h"
#include "models/axon.h"
/**
 * @brief Creates a tonic neuron
 * @details From the \cite Cassidy2013.101045 paper, this
 * neuron spikes multiple times.
 *
 * @param s Neuron state
 * @param lp lp
 */
void crTonic(neuronState *s, tw_lp *lp);
/**
 * @brief Creates a phasic spiking neuron
 * @details Phasic spiking neuron - spikes once, with input
 * starting from zero, and extending to one per time slice.
 * For the purposes of this test, the neuron outputs to a
 * disconnected synapse
 *
 * @param s Neuron state
 * @param lp lp
 *  */
void crPhasic(neuronState *s, tw_lp *lp);
void crTonicBursting(neuronState *s, tw_lp *lp);
void crMixedMode(neuronState *s, tw_lp *lp);
void crFrequencyAdaptation(neuronState *s, tw_lp *lp);
void crClassOneExciteable(neuronState *s, tw_lp *lp);
void crClassTwoExciteable(neuronState *s, tw_lp *lp);
/**
 * @brief Spike Latency Neuron
 * @details Creates a spike latency neuron, where one neuron
 * spikes after a delay from input.
 * For the purposes of this test, the neuron outputs to a
 * disconnected synapse
 * @param s Neuron
 * @param lp An LP state
 */
void crSpikeLatency(neuronState *s, tw_lp *lp);
void crSubthresholdOsc(neuronState *s, tw_lp *lp);
void crResonator(neuronState *s, tw_lp *lp);
void crIntegrator(neuronState *s, tw_lp *lp);
void crReboundSpike(neuronState *s, tw_lp *lp);
void crReboundBurst(neuronState *s, tw_lp *lp);
void crThresholdVariability(neuronState *s, tw_lp *lp);
void crBistability(neuronState *s, tw_lp *lp);
void crDepolarizingAfterPotent(neuronState *s, tw_lp *lp);
void crAccomodation(neuronState *s, tw_lp *lp);
void crInhibitionInducedSpiker(neuronState *s, tw_lp *lp);
void crInhibitionInducedBurst(neuronState *s, tw_lp *lp);

void crPhasicAxon(axonState *s, tw_lp *lp);
void crTonicBurstingAxon(axonState *s, tw_lp *lp);
void crBioLoopback(neuronState *s, tw_lp *lp);
bool *identityConnectivity(int localID);
bool *zeroIDMatrix(int n, int m);

#endif /*  bioModel_h */
