//
// Created by Mark Plagge on 9/4/17.
//

#include "tomacs_exp.h"

/** \ingroup synapSat @{ */


//! Synaptic connectivity method. 0=Per axon, 1= Per Core
int synConMeth = 0;
int *synCoreBucket;

//SAT NET parameters
//unsigned int SAT_NET_PERCENT;
//bool SAT_NET_COREMODE ;
//unsigned int SAT_NET_THRESH;
//unsigned int SAT_NET_LEAK;
//bool SAT_NET_STOC;
//bool IS_SAT_NET;

//! Neurons have connectivityProb probability of having a connection.
float connectivityProb = 0.2 ; // (float) SAT_NET_PERCENT / 100;
//! when connected,  axons will have this weight
int connectedWeight = 1;
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

	}
	if(cCore != lastCore || isInit == 0){
		int ctr = 0;
        bool err = true;
        if (isInit != 0) {
            if (cCore != lastCore + 1) {
                tw_printf(TW_LOC, "CB err - new core is not incremental: ");

            } else if (cCore < lastCore) {
                tw_printf(TW_LOC, "CB err - new core is `earlier` than oldCore: ");
            } else {
                err = false;
            }

            if (err) {
                tw_printf(TW_LOC, "LastCore: %lu NewCore %lu", lastCore, cCore);
                tw_printf(TW_LOC, "*****************************\n");
            }
        }else{
            isInit = 1;
        }

		long long pbr = (int)(floorf(synCoreBktSize * ((float)SAT_NET_PERCENT / 100.0)));
        //or for pure stochastic mode:
		for (int i = 0; i < synCoreBktSize; i ++){
			synCoreBucket[i] = i < pbr;
		}
		shuffle(synCoreBucket,synCoreBktSize,lp);
		lastCore = cCore;
		idx = 0;
//        long double tmp = 0.0;
//        for(int i = 0; i < synCoreBktSize; i ++){
//            tmp += synCoreBucket[i];
//        }
//        tmp /= synCoreBktSize;
//        tw_printf(TW_LOC, "bucket created with %Lf connections vs %i %% calculated.", tmp, SAT_NET_PERCENT);

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

	if(SAT_NET_COREMODE){
		getCoreConnBkt(lp,synapticConGrid);

	}else {
		//per axon method
		for(int i = 0; i < AXONS_IN_CORE; i++){
			synapticConGrid[i] = tw_rand_unif(lp->rng) > connectivityProb;
		}

	}

}

/** @} */

