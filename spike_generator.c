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
		if(tw_rand_unif(str) < st->rndSpikes.randomRate / 100 )
		willFire = true;
	return willFire;
}
