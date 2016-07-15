#include "globals.h"

int iIABS(int vals){

        int result;
        asm ("movl  %[valI], %%eax;"
                "cdq;"
                "xor %%edx, %%eax;"
                "sub %%edx, %%eax;"
                "movl %%eax, %[resI];"
        : [resI] "=r" (result)
        : [valI] "r" (vals)
        : "cc","%eax", "%ebx");
        return result;
    return -1;


}



/** Faster version  of IABS (no branching) but needs types. @todo this
 method will be faster on BGQ, but need to make sure that it works properly */
//  weight_type IABS(weight_type in){
//     //int_fast64_t const mask = in >> sizeof(int_fast64_t) * CHAR_BIT - 1;
//     #ifdef HAVE_SIGN_EXTENDING_BITSHIFT
//     int const mask = v >> sizeof(int) * CHAR_BIT - 1;
//     #else
//     int const mask = -((unsigned)in >> sizeof(int) * CHAR_BIT - 1);
//     #endif
//     return (in ^ mask) - mask;
// }


/**
 * bigTickRate is the rate of simulation - neurons synchronize based on this value.
 */
tw_stime bigTickRate = 1;
/**
 * littleTick is the rate of intra core synchronization. Synapses and axons do 
 * calculations using this rate, and neurons integrate with messages set to this rate.
 */
tw_stime littleTick = 0.001;



/**
 * @brief      Gets the next event time. This is a jitter
 *
 * @param      lp    The pointer to a lp.
 *
 * @return     The next event time. Is a jitter value that is < 0.001.
 */
tw_stime getNextEventTime(tw_lp *lp) {
    return (tw_rand_unif(lp->rng) / 1000)  + littleTick;
}


/**
 * Gets the next event time for synapse internal heartbeats - 
 
 */

tw_stime getSynapseHeartbeatTime(tw_lp *lp){
	return (tw_rand_unif(lp->rng) / 1000) + bigTickRate;
}


tw_stime getCurrentBigTick(tw_stime now){
  return floor(now);
}


tw_stime getNextBigTick(tw_lp *lp, tw_lpid neuronID) {    
    return (tw_rand_unif(lp->rng) / 1000) + bigTickRate;
}