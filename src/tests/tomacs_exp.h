//
// Created by Mark Plagge on 9/4/17.
//

#ifndef SUPERNEMO_TOMACS_EXP_H
#define SUPERNEMO_TOMACS_EXP_H
#import "../globals.h"
#import "../neuro/tn_neuron.h"
#import "ross.h"
#ifdef _OPENMP
#include <omp.h>
#else


/** \defgroup synapSat Synaptic Saturation Experement @{ */

int connectedWeight;

//not used as a temp variable swap method is actually faster on modern CPU compiler combos
//#define SWAP(a, b) (((a) == (b)) || (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))))


/**
 * using the connectivity probability, this function returns a connectivity grid for a neuron.
 * Using a bucket method instead of true random will help with the reandomness.
 * @param synapticConGrid
 *
 */
void getSynapticConnectivity(bool *synapticConGrid, tw_lp *lp);

void clearBucket();


/**@}*/

/** \defgroup convNet Simple Convolutional network experement @{ /*/



/** @} */
#endif
#endif //SUPERNEMO_TOMACS_EXP_H
