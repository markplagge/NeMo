//
// Created by Mark Plagge on 9/4/17.
//

#include "tomacs_exp.h"

/** \ingroup synapSat @{ */
//! Neurons have connectivityProb probability of having a connection.
float connectivityProb = 0.2;
//! when connected,  axons will have this weight
int connectedWeight = 1;

//! Synaptic connectivity method. 0=Per axon, 1= Per Core
int synConMeth = 0;
int *synCoreBucket;

weight_type alpha = 4;
weight_type leakValue = 1;


/**
 * Fisher-Yates Shuffle
 * @param pool
 * @param n
 * @param lp
 *
 */
void shuffle(int *pool, unsigned long n, tw_lp *lp){
	long j;

	for (unsigned long i = n-1; i > 0; i--){
		j = tw_rand_integer(lp->rng,0,i);
		int x = pool[i];
		pool[i] = pool[j];
		pool[j] = x;
	}


}

void  getCoreConnBkt(tw_lp *lp, bool * coreConArr){
	static int isInit = 0;
	static id_type lastCore = 0;
	static int idx = 0;
	id_type cCore = getCoreFromGID(lp->gid);

	unsigned long synCoreBktSize = NEURONS_IN_CORE * AXONS_IN_CORE;
	if (isInit == 0) {
		synCoreBucket = tw_calloc(TW_LOC, "bucketData", sizeof(int), synCoreBktSize);
		isInit = 1;
	}
	if(cCore != lastCore){
		int ctr = 0;

		long long pbr = (int)(floorf(synCoreBktSize * connectivityProb));
		for (int i = 0; i < synCoreBktSize; i ++){
			synCoreBucket[i] = i > pbr;
		}
		shuffle(synCoreBucket,synCoreBktSize,lp);
		lastCore = cCore;
		idx = 0;

	}
	for(int i = 0; i < AXONS_IN_CORE; i++){
		coreConArr[i] = (bool)synCoreBucket[idx];
		idx ++;
	}


}
void clearBucket(){
	free(synCoreBucket);
}
void getSynapticConnectivity(bool *synapticConGrid, tw_lp *lp) {

	if(synConMeth == 1){
		getCoreConnBkt(lp,synapticConGrid);

	}else {
		//per axon method
		for(int i = 0; i < AXONS_IN_CORE; i++){
			synapticConGrid[i] = tw_rand_unif(lp->rng) > connectivityProb;
		}

	}

}
void TN_create_saturation_neuron(tn_neuron_state* s, tw_lp* lp) {
	TN_init(s,lp);
	static int created = 0;
	bool synapticConnectivity[NEURONS_IN_CORE];
	short G_i[NEURONS_IN_CORE];
	short sigma[4] = {1};
	short S[4] = {[0] = 3};
	bool b[4] = {0};
	bool epsilon = 0;
	bool sigma_l = 0;
	short lambda = 0;
	bool c = false;
	short TM = 0;
	short VR = 0;
	short sigmaVR = 1;
	short gamma = 0;
	bool kappa = 0;
	int signalDelay = 1;
	weight_type beta = -1;
	getSynapticConnectivity(&synapticConnectivity,lp);

	for (int i = 0; i < NEURONS_IN_CORE; i++) {
		// s->synapticConnectivity[i] = tw_rand_integer(lp->rng, 0, 1);
		s->axonTypes[i] = 1;
		s->synapticConnectivity[i] = synapticConnectivity[i];

	}
	for (int i = 0; i < NUM_NEURON_WEIGHTS; i ++){
		s->synapticWeight[i] = connectedWeight;

	}



}
/** @} */

