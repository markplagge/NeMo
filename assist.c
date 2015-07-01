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

        //tw_stime r =tw_rand_unif(lp->rng);

  tw_stime r = tw_rand_exponential(lp->rng, 1)/10;
        return r;

}
void setBigLittleTick(){

      littleTick =  (NEURONS_IN_CORE + 1);// /g_tw_clock_rate;


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



	//long double vtr = 0;
	//long long rem = modfl(ctick, &vtr);
	//Rem is current tick, vtr is offset.
	double rounded =-floor(now);
	double tmp;
	long tt = rounded;
	tt = abs(tt);

	tw_stime rem = (tt / (unsigned long)bigTickRate) ;//* bigTickRate ;

	return rem;


}
        //@todo This does not work - need to whiteboard it to figure out the conversion.
tw_stime getNextBigTick(tw_stime now) {
  if(littleTick == 0 || bigTickRate == 0)
    setBigLittleTick();

       //tw_stime nextTickTime = getCurrentBigTick(now) + g_tw_clock_rate;
        tw_stime inter;
        tw_stime nbtd = bigTickRate-modf(now,&inter);
        //nbtd += g_tw_clock_rate;
        return nbtd;
        //return 2;

                //Need to figure this out - not accurate until this is done:

}


int testTiming(){
//test timing with the model params:
  tw_lp* rap = tw_getlp(0);
  //Pretend all axons get a message first:
  int tester = 0;
  printf("%i synapses in core\n", SYNAPSES_IN_CORE);
  while (tester < 4){

  tw_stime axonSendTime[AXONS_IN_CORE];
  for(int i = 0; i < AXONS_IN_CORE; i ++) {
       int adj = tester * SYNAPSES_IN_CORE;
      axonSendTime[i] = getNextEventTime(rap) + adj;
    }

  //next, check the first synapse layer:
  tw_stime firstNeuronOutTime[AXONS_IN_CORE];
  for(int i = 0; i < AXONS_IN_CORE; i ++) {
      firstNeuronOutTime[i] = getNextBigTick(axonSendTime[i]);
    }


  //See if this round of big ticks makes sense:

  printf( "Axons in core: %i - Synapses in Core %i. \n",AXONS_IN_CORE, NEURONS_IN_CORE);
for(int i = 0; i < AXONS_IN_CORE; i ++){
    printf("Ax/Ne %i Sends from %f -> Neuron @ %f\n", i, axonSendTime[i],firstNeuronOutTime[i]);
  }
printf("\n");
printf("Current big tick from little tick %f is %f\n", axonSendTime[0],getCurrentBigTick(axonSendTime[0]));
tester ++;

    }
exit(0);

}
