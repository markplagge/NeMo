//
//  assist.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/17/15.
//
//

#include "assist.h"
/**
 *  Gets the next event time, based on a random function. Moved here to allow for
 *	easier abstraciton, and random function replacement.
 *
 *
 *  @param lp Reference to the current LP so that the function can see the RNG
 *
 *  @return a tw_stime value, such that \f$ 0 < t < 1 \f$. A delta for the next
 *  time slice.
 */
tw_stime getNextEventTime(tw_lp *lp){

	tw_stime r =tw_rand_unif(lp->rng) / 10;

	return r;

}