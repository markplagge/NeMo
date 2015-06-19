//
//  input_simulator.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#include "input_simulator.h"
bool uniformGen(void *spikeGen, tw_lp *lp) {
	bool willFire = false;

	inputSimulatorState * st = (inputSimulatorState * ) spikeGen;
	tw_rng_stream *str = (tw_rng_stream *) lp->rng;
	if(tw_rand_unif(str) < st->randomRate)
		willFire = true;
	return willFire;
}