//
//  assist.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/17/15.
//
//

#include "assist.h"
        #include <math.h>
///Big-tick offset - this is the delta for big ticks (neuron events)
tw_stime bigTickRate = 0;

/**
 *  Gets the next small-tick event time.
 */
tw_stime getNextEventTime(tw_lp *lp) {

  tw_stime bigTickRate = 0;
  tw_stime r; 
  unsigned int ct = 0;
  switch(CLOCK_RND_MODE) {
    case RND_UNF:
  r =tw_rand_unif(lp->rng);
  break;
  case RND_NORM_BASED:
  r = tw_rand_normal_sd(lp->rng, CLOCK_RANDOM_ADJ, 20,&ct);
  break;
  case RND_EXP:
  r = tw_rand_exponential(lp->rng, CLOCK_RANDOM_ADJ) ;
		  break;
  default:
  r = tw_rand_binomial(lp->rng, 100, CLOCK_RANDOM_ADJ);
  lp->rng->count += ct;

}
  r *= littleTick;
  return r;
}
void setBigLittleTick() {
  
  bigTickRate = ceill(littleTick);
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
  if(littleTick == 0 || bigTickRate == 0)
    setBigLittleTick();


  return floor(now);

}
        //@todo Seems to work just fine - but need to double check.
tw_stime getNextBigTick(tw_stime nextEventTime) {
  if(littleTick == 0 || bigTickRate == 0)
    setBigLittleTick();

  return 10 + nextEventTime;
                //Need to figure this out - not accurate until this is done:

}

int testTiming() {
  // test timing with the model params:
  tw_lp *rap = tw_getlp(0);
  // Pretend all axons get a message first:
  int tester = 0;
  printf("%i synapses in core\n", SYNAPSES_IN_CORE);
  tw_stime *firstNeuronOutTime;  //[AXONS_IN_CORE];
  firstNeuronOutTime = calloc(sizeof(tw_stime), AXONS_IN_CORE);
  tw_stime *axonSendTime;
  axonSendTime = calloc(sizeof(tw_stime), AXONS_IN_CORE);

  while (tester < 1024) {
    for (int i = 0; i < AXONS_IN_CORE; i++) {
      axonSendTime[i] = getNextEventTime(rap) + firstNeuronOutTime[i];
    }

    // next, check the first synapse layer:

    for (int i = 0; i < AXONS_IN_CORE; i++) {
      firstNeuronOutTime[i] = getNextBigTick(axonSendTime[i]);
    }

    // See if this round of big ticks makes sense:

    printf("Axons in core: %i - Synapses in Core %i. \n", AXONS_IN_CORE,
           NEURONS_IN_CORE);
    for (int i = 0; i < AXONS_IN_CORE; i++) {
      double xv = axonSendTime[i];
      if (xv < 0) printf("Less than zero\n");
      printf("Ax/Ne %i Sends from %f -> Neuron @ %f\n", i, axonSendTime[i],
             firstNeuronOutTime[i]);
    }
    printf("\n");
    printf("Current big tick from little tick %f is %f\n", axonSendTime[0],
           getCurrentBigTick(axonSendTime[0]));
    tester++;
  }
  exit(0);
}
