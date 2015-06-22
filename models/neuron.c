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
 *  @details LinearLeak is the standard linear leak function from the paper. Using the leakRate parameter inside \link NeuronModel the neuron state \endlink, this follows a simple linear function, reducing the membrane potential by a fixed rate per tick.
 *

 *	@see NeuronModel
 */
void linearLeak(void *neuron, tw_stime now) {
	neuronState *s = (neuronState *)neuron;
	tw_stime bigTick = getCurrentBigTick(now);
	tw_stime delta = bigTick - s->lastLeakTime;

		//s->membranePot -= s->leakRate * delta;
	s->lastLeakTime = now;
}

void revLinearLeak(void *neuron, tw_stime now){
	neuronState *s = (neuronState *)neuron;
	tw_stime delta = s->lastLeakTime - now;

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
 *  Big-Tick events are sent at time \f$𝒯_{bigTick} - ε\f$, so that the the event will arrive at the next bit-tick time.
 * @todo remove delta encoding
 *
 */
void neuronReceiveMessage(neuronState *st, tw_stime time, Msg_Data *m,tw_lp *lp){

  //Delta encoding - used for now until reverse event computation can be checked for accuracy.
  long delta_size;


  // This should be the FIRST thing to do in your event handler
  if (g_tw_synchronization_protocol == OPTIMISTIC ||
      g_tw_synchronization_protocol == OPTIMISTIC_DEBUG) {
    // Only do this in OPTIMISTIC mode
    tw_snapshot(lp, lp->type->state_sz);
  }
        //random fn call state management.
        unsigned long startCount = lp->rng->count;

	//state management
	bool willFire = false;
	st->firedLast = false;
		//save the previous state of the neuron:
	st->savedLastLeakTime = st->lastLeakTime;
	st->savedLastActiveTime = st->lastActiveTime;
	st->savedMembranePot = st->membranePot;



	switch (m->eventType) {
  case SYNAPSE_OUT:
			integrateSynapse(m->localID, st, lp);

				//next, we will check if a heartbeat message should be sent
			if(st->receivedSynapseMsgs == 0) {
				sendHeartbeat(st, lp, time);
			}
			st->receivedSynapseMsgs ++;


			break;

		case NEURON_HEARTBEAT:
			st->receivedSynapseMsgs = 0;
				//set up drawn random number for the heartbeat.
			if(st->thresholdPRNMask != 0)
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


	//Delta Encoding
	///@todo remove this to enhance performance, once reverse computation is correct.
	// This should be the LAST thing you do in your event handler
	// (Take care to cover all possible exits!)
	if (g_tw_synchronization_protocol == OPTIMISTIC ||
	    g_tw_synchronization_protocol == OPTIMISTIC_DEBUG) {


	  delta_size = tw_snapshot_delta(lp, lp->type->state_sz);

	  //removed average. Code comes lifted from  dphold.c

	  m->rndCallCount= lp->rng->count - startCount;

	}



}

bool neuronShouldFire(neuronState *st, tw_lp *lp){
		//check negative threshold values:

	return st->membranePot >= st->threshold + st->drawnRandomNumber;
}

void nSpike(neuronState *st, tw_stime time, tw_lp *lp){
	tw_stime nextHeartbeat = getNextBigTick(time);
	tw_event *newEvent = tw_event_new(st->dendriteGlobalDest, nextHeartbeat, lp);
	Msg_Data *data = (Msg_Data *) tw_event_data(newEvent);
	data->eventType = NEURON_OUT;
	data->localID = st->myLocalID;
	tw_event_send(newEvent);
	st->firedLast = true;
}



void sendHeartbeat(neuronState *st, tw_lp *lp, tw_stime time){
		//heartbeat message gen:
	tw_stime nextHeartbeat = getNextBigTick(time);
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
void stochasticIntegrate(_weightT weight, neuronState *st, tw_lp *lp){

	long drawnRandom = st->drawnRandomNumber;
	if (BINCOMP(weight, drawnRandom)){
		st->membranePot += SGN(weight);
	}
}


void integrateSynapse(_idT synapseID,neuronState *st, tw_lp *lp) {

	if(st->synapticWeightProbSelect[synapseID] == true) {
		stochasticIntegrate(st->synapticWeightProb[synapseID], st, lp);
	} else { //det. mode integrate:
		_voltT adjustedWeight = st->synapticWeightProb[synapseID];
		st->membranePot += adjustedWeight;
	}


}
/** @todo There is potentially an issue here - does the TrueNorth architecture draw a new
 *	pseudorandom number during the threshold, fire, reset functions, or does it resuse them? It
 *	looks like re-use, so that's what I'm doing here.
 */
void neuronPostIntegrate(neuronState *st, tw_stime time, tw_lp *lp, bool willFire){
	if(willFire){ // neuron will/did fire:
		st->doReset(st);
	} else if (st->membranePot < -1 * (st->negativeThreshold * st->negThresReset + (st->negativeThreshold + st->drawnRandomNumber))){
			//sanity variables for the formulaic reset/bounce instead of calling functions:
		_threshT B = st->negativeThreshold;
		int K = st->negThresReset;
		int G = st->resetMode;
		_randT n = st->drawnRandomNumber;
		_voltT R = st->resetVoltage;
		_voltT V = st->membranePot;
		st->membranePot = (-(B*K) + (-(DT(G)) * R +
					     DT(G-1) * (V + (B + n)) +
					     DT(G-2) * V) * (1-K));

	}

}
void  neuornReverseState(neuronState *s, tw_bf *CV,Msg_Data *m,tw_lp *lp) {
		//reverse function.
    long count = m->rndCallCount;
    /** @todo - check this for correctness and switch from delta encoding. *//*
  if(M->eventType == SYNAPSE_OUT)
		s->receivedSynapseMsgs --;
	else
		s->SOPSCount --;

	if(s->firedLast == true){
		s->fireCount --;
	}

	s->membranePot  = s->savedMembranePot;
	s->lastActiveTime = s->savedLastActiveTime;
	s->lastLeakTime = s->savedLastLeakTime;*/

	// This should be the FIRST thing to do in your reverse event handler
	tw_snapshot_restore(lp, lp->type->state_sz, lp->pe->cur_event->delta_buddy, lp->pe->cur_event->delta_size);
	while (count--) {
	    tw_rand_reverse_unif(lp->rng);
	}


}
/** @} */
