//
//  assist.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/17/15.
//
//

#include "assist.h"
	#include <math.h>
///number of simulation ticks per big tick (determined by the g_tw_clock_rate param)
tw_stime littleTick = 0;
tw_stime bigTickRate = 0;
/**
 *  Gets the next small-tick event time.
 */
tw_stime getNextEventTime(tw_lp *lp){

	tw_stime r =tw_rand_unif(lp->rng) / 100;

	return r;

}
/**
 *  @details  If the time is in-between
 *  big ticks, this rounds down to the last big tick. There is a bit of a fuzz for
 *  times close to the next big tick so if the current time is within  ::BIG_TICK_ERR
 *  of the next big tick, that will be returned instead. Sane parameters would
 *  probably be around .000001. @todo: Implement & determine if ε needs to be added
 *  to the return value.

 */
tw_stime getCurrentBigTick(tw_stime now){
  if(littleTick == 0 ){
      littleTick = g_tw_clock_rate / (SYNAPSES_IN_CORE + 1);
    }
  if(bigTickRate == 0){
      bigTickRate = ceill(littleTick) + 1;
    }


	//long double vtr = 0;
	//long long rem = modfl(ctick, &vtr);
	//Rem is current tick, vtr is offset.
	tw_stime rem = bigTickRate - now;

	return floorl(rem);


}
	//@todo This does not work - need to whiteboard it to figure out the conversion.
tw_stime getNextBigTick(tw_stime now) {
  if(littleTick == 0 ){
      littleTick = g_tw_clock_rate / (SYNAPSES_IN_CORE + 1);
    }
  if(bigTickRate == 0){
      bigTickRate = ceill(littleTick) + 1;
    }
        tw_stime nextTickTime = getCurrentBigTick(now) + g_tw_clock_rate;
        tw_stime nbtd = nextTickTime-now;
        //nbtd += g_tw_clock_rate;
        return nbtd;

		//Need to figure this out - not accurate until this is done:

}
