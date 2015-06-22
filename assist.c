//
//  assist.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/17/15.
//
//

#include "assist.h"
	#include <math.h>
/**
 *  Gets the next small-tick event time.
 */
tw_stime getNextEventTime(tw_lp *lp){

	tw_stime r =tw_rand_unif(lp->rng) / 10;

	return r;

}
/**
 *  @details  If the time is in-between
 *  big ticks, this rounds down to the last big tick. There is a bit of a fuzz for
 *  times close to the next big tick so if the current time is within  ::BIG_TICK_ERR
 *  of the next big tick, that will be returned instead. Sane parameters would
 *  probably be around .000001. @todo: Implement & determin if ε needs to be added
 *  to the return value.

 */
tw_stime getCurrentBigTick(tw_stime now){
	tw_stime ctick = now / CORE_SIZE;
		//!@todo need to see if this will kill performance:
	long double vtr = 0;
	long long rem = modfl(ctick, &vtr);
	//Rem is current tick, vtr is offset.

	return rem;

}
	//@todo This does not work - need to whiteboard it to figure out the conversion.
tw_stime getNextBigTick(tw_stime now) {
	long long curr = CORE_SIZE * getCurrentBigTick(now) + 1;
	return curr - now;

		//Need to figure this out - not accurate until this is done:

}