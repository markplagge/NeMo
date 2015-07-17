//
//  neuron.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#include "neuron.h"

/** @name LeakFunctions
 * Neuron functions that manage leaks. All voltage state saving
 * must be handled in the neuron event function neuronReceiveMessage().
 * @{
 */
void noLeak(void *neuron, tw_stime now) {
  // do nothing!!
}
void revNoLeak(void *neuron, tw_stime now) {
  // do nothing!!
}
/**
 *  @details LinearLeak is the standard linear leak function from the paper.
 * Using the leakRate parameter inside \link NeuronModel the neuron state
 * \endlink, this follows a simple linear function,
 * reducing the membrane potential by a fixed rate per tick.
 * This calculates based on the previous time a neuron leaked (on the big tick),
 * and uses the difference to calculate how much to leak.
 *
 *	@see NeuronModel
 */
void linearLeak(void *neuron, tw_stime now) {
	neuronState *s = (neuronState *)neuron;
	tw_stime bigTick = getCurrentBigTick(now);

	tw_stime delta = bigTick - s->lastLeakTime;


	//if the leak is probabilistic:
	if(s->leakWeightProbSelect){
	    while(delta > 0){
		stochasticIntegrate(s->leakRateProb,s);
		delta --;
	      }
	  }
	else {
	     short int omega = (1 - s->leakReversalFlag) + s->leakReversalFlag *SGN(s->membranePot);
	     s->membranePot += omega * s->leakRateProb;

	  }


	s->lastLeakTime = bigTick;
}

void revLinearLeak(void *neuron, tw_stime now){
	neuronState *s = (neuronState *)neuron;
		//tw_stime delta = s->lastLeakTime - now;

	//s->membranePot += s->leakRate * delta;
	s->lastLeakTime = now;
}



/** @}*/
/** @name ResetFunctions
 Reset function defs. Neuron reset functions will
 change the neuron state without saving the previous state. All voltage state saving
 must be handled in the neuron event function neuronReceiveMessage().
 @todo: Check that reverse reset functions are needed, since previous voltage is stored in the neuron.
 * @{ */


void resetNormal(void *neuronState) {
	struct NeuronModel *s = (struct NeuronModel *)neuronState;
	s->membranePot = s->resetVoltage; // set current voltage to \f$R\f$.
}

/**
 *  @details Linear reset mode - ignores \f$R\f$, and sets the membrane potential
 *  to the difference between the threshold and the potential.
 *
 */
void resetLinear(void *neuronState) {
	struct NeuronModel *s = (struct NeuronModel *)neuronState;
    s->membranePot = s->membranePot - s->threshold;
}

/// @todo This might not be needed, since we are saving voltage state.
/// \brief reverseResetLinear
/// \param neuronState
///
void reverseResetLinear(void *neuronState){

	struct NeuronModel *s = (struct NeuronModel *)neuronState;
	_voltT resetParam = (_voltT) s->resetVoltage
	    ;
	s->membranePot = s->membranePot + resetParam;
}


/** @todo: Check that this is the proper way to handle reset zero function */
void reverseResetNormal(void *neuronState){

}


void resetNone(void *neuronState){
}
void reverseResetNone(void *neuronState) {
}
        /** @} */

/** @name NeuronFunctions
* Main neuron functions and behaviors
*  @{
 */

/**
 *  @details  neruonReceiveMessage runs when an event is received by a neuron.
 *  The events are handled in the following fashion:
 *  - If the event is a synapse, the neuron integrates the weight of the synapse into its membrane potential.
 *    - The count of big-tick synapse messages recived is incremented.
 *    - If the count changes from zero to one, the function creates and queues a heartbeat event for the next big-tick.
 *  - If the event is a heartbeat, the neuron will leak, potentially fire, and reset.
 *
 *  Big-Tick events are sent at time \f$𝒯_{bigTick} + ε\f$, so that the the event will arrive at the next bit-tick time.
 * @todo remove delta encoding
 *
 */
void neuronReceiveMessage(neuronState *st, tw_stime time, Msg_Data *m,tw_lp *lp){


  
        //state management
	bool willFire = false;
	st->firedLast = false;
		//save the previous state of the neuron:
		//st->savedLastLeakTime = st->lastLeakTime;
		//st->savedLastActiveTime = st->lastActiveTime;
		//st->savedMembranePot = st->membranePot;
	m->neuronVoltage = st->membranePot;
	m->neuronLastLeakTime = st->lastLeakTime;
	m->neuronLastActiveTime = st->lastActiveTime;



	switch (m->eventType) {
	  case SYNAPSE_OUT:
                        integrateSynapseFast(m->axonID, st, lp);

				//next, we will check if a heartbeat message should be sent
			if(st->receivedSynapseMsgs == 0) {
				sendHeartbeat(st, lp, time);
			}
			st->receivedSynapseMsgs ++;


			break;
	  case NEURON_HEARTBEAT:
			st->receivedSynapseMsgs = 0;
				//set up drawn random number for the heartbeat.
			//if(st->thresholdPRNMask != 0)
			st->drawnRandomNumber = tw_rand_ulong(lp->rng, 0, ULONG_MAX) & st->thresholdPRNMask;

				//Currently operates - leak->fire->(reset)

			st->doLeak(st, time);
			willFire = neuronShouldFire(st,lp);
			if(willFire){
				nSpike(st, time, lp);
				st->fireCount ++;
			}

			neuronPostIntegrate(st, time, lp, willFire);
			//stats collection
			st->SOPSCount ++;
			st->lastActiveTime = tw_now(lp);
			break;
	  default:
                                //Error condition - non-valid input.
                        break;
        }

		//store the random calls in the message:


	st->rcvdMsgCount ++;






}
//neuron is in basic firing mode with delta encoding.
void neuronReceiveMessageBasic(neuronState *st, tw_stime time, Msg_Data *m, tw_lp *lp){
	 unsigned long startCount = lp->rng->count;
	m->neuronVoltage = st->membranePot; //save the state.
	m->neuronLastActiveTime = tw_now(lp); //simple mode does not use the last time
	m->neuronLastLeakTime = tw_now(lp); //but I save them here so the normal reverse function will work.
    switch (m->eventType) {
        //random fn call state management.

        //state management
        bool willFire = false;
        st->firedLast = false;
      case SYNAPSE_OUT:
                    //basic integrate function:
			integrateSynapseFast(m->axonID, st, lp);

                            //next, we will check if a heartbeat message should be sent
                    if(st->receivedSynapseMsgs == 0) {
                            sendHeartbeat(st, lp, time);
                    }
                    st->receivedSynapseMsgs ++;


                    break;
        case NEURON_HEARTBEAT:
                      st->receivedSynapseMsgs = 0;
                      willFire = st->membranePot >= st->threshold;
                      if(willFire){
                              nSpike(st, time, lp);
                              st->fireCount ++;
                              st->membranePot = 0;
                      }


                      //stats collection
                      st->SOPSCount ++;
                      st->lastActiveTime = tw_now(lp);
                      break;
        default:
                              //Error condition - non-valid input.
                      break;
      }
	st->rcvdMsgCount ++;
	m->rndCallCount= lp->rng->count - startCount;

}

bool neuronShouldFire(neuronState *st, tw_lp *lp){
                //check negative threshold values:
  return st->membranePot >= st->threshold + (st->drawnRandomNumber & st->thresholdPRNMask);


}

void nSpike(neuronState *st, tw_stime time, tw_lp *lp){
	tw_stime nextHeartbeat = getNextBigTick(lp);
	tw_event *newEvent = tw_event_new(st->dendriteGlobalDest, nextHeartbeat, lp);
	Msg_Data *data = (Msg_Data *) tw_event_data(newEvent);
	data->eventType = NEURON_OUT;
	data->localID = st->myLocalID;
	tw_event_send(newEvent);
	st->firedLast = true;
}



void sendHeartbeat(neuronState *st, tw_lp *lp, tw_stime time){


                //random fn call state management.
  //printf("heartbeat sent \n");

	tw_stime nextHeartbeat = getNextBigTick(lp);
	tw_event *newEvent = tw_event_new(lp->gid, nextHeartbeat, lp);
	Msg_Data *data = (Msg_Data *) tw_event_data(newEvent);
	data->eventType = NEURON_HEARTBEAT;
	data->localID = st->myLocalID;

	tw_event_send(newEvent);


}
/**
 * @details From the Cassidy 2013 paper, page 4, Part D. This function covers the
 * stochastic synaptic and leak integration functions defined in the paper.
 * @param weight weight - can be considered as both \f$
 * @param st
 * @param lp
 */
void stochasticIntegrate(_weightT weight, neuronState *st){

	long drawnRandom = st->drawnRandomNumber;
	if (BINCOMP(weight, drawnRandom)){
		st->membranePot += SGN(weight);
	}
}


	//non huge array method of integration.
void integrateSynapseFast(_idT axonID, neuronState *st, tw_lp *lp) {
	_voltT adjWt = st->axonWeightProb[st->weightSelect[axonID]];
	if(st->axonProbSelect[st->weightSelect[axonID]]){
		stochasticIntegrate(adjWt, st);
	} else {
		st->membranePot += adjWt;
	}



}

/** @todo There is potentially an issue here - does the TrueNorth architecture draw a new
 *	pseudorandom number during the threshold, fire, reset functions, or does it resuse them? It
 *	looks like re-use, so that's what I'm doing here.
 */
void  neuronPostIntegrate(neuronState *st, tw_stime time, tw_lp *lp, bool willFire){
	if(willFire){ // neuron will/did fire:
		st->doReset(st);
	} else if (st->membranePot < -1 * (st->negativeThreshold * st->negThresReset + (st->negativeThreshold + st->drawnRandomNumber))){
			//sanity variables for the formulaic reset/bounce instead of calling functions:
		_threshT B = st->negativeThreshold;
		int K = st->negThresReset;
		int G = st->resetMode;
		_randT n = st->drawnRandomNumber & st->thresholdPRNMask;
		_voltT R = st->resetVoltage;
		_voltT V = st->membranePot;
		st->membranePot = (-(B*K) + (-(DT(G)) * R +
					     DT(G-1) * (V + (B + n)) +
					     DT(G-2) * V) * (1-K));

	}

}
void  neuronReverseState(neuronState *s, tw_bf *CV,Msg_Data *m,tw_lp *lp) {

                //reverse function.
    long count = m->rndCallCount;

    /** @todo - check this for correctness and switch from delta encoding. */
  if(m->eventType == SYNAPSE_OUT){
                s->receivedSynapseMsgs --;
    }else if(m->eventType == NEURON_HEARTBEAT){
      s->SOPSCount --;
    }

	if(s->firedLast == true){
		s->fireCount --;
		s->firedLast = false;
	}

	s->membranePot = m->neuronVoltage ;
	s->lastLeakTime =  m->neuronLastLeakTime;
	s->lastActiveTime = m->neuronLastActiveTime;

	while (count--) {
	    tw_rand_reverse_unif(lp->rng);
	}


}
/** @} */
