//
//  spike_generator.c
//  TNT_MAIN
//
//  Created by Mark Plagge on 4/14/15.
//
//

#include "spike_generator.h"

bool uniformGen(void *spikeGen, tw_lp *lp) {
	bool willFire = false;
	
	spikeGenState * st = (spikeGenState * ) spikeGen;
	tw_rng_stream *str = (tw_rng_stream *) lp->rng;
		if(tw_rand_unif(str) < st->randomRate / 100 )
		willFire = true;
	return willFire;
}



bool expGen(void *gen_state, tw_lp *lp)
{
    return false; //Decided by a fair coin flip.
}
