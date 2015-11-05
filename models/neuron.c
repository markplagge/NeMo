//
//  neuron.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#include "neuron.h"


/** Constructor / Init a new neuron */
void initNeuron(id_type coreID, id_type nID,
                bool synapticConnectivity[NEURONS_IN_CORE],
                short G_i[NEURONS_IN_CORE], int sigma[4],
                int S[4], bool b[4], bool epsilon,
                short sigma_l, short lambda, bool c, uint32_t alpha,
                uint32_t beta, short TM, short VR, short sigmaVR, short gamma,
                bool kappa, neuronState *n, int signalDelay, uint64_t destGlobalID,int destAxonID)
{
    
    for(int i = 0; i < 4; i ++) {
        n->synapticWeight[i] = sigma[i] * S[i];
        n->weightSelection[i] = b[i];
    }
    for(int i = 0; i < NEURONS_IN_CORE; i ++) {
        n->synapticConnectivity[i] = synapticConnectivity[i];
        n->axonTypes[i] = G_i[i];
    }
    
    //set up other parameters
    n->myCoreID = coreID;
    n->myLocalID = nID;
    n->epsilon = epsilon;
    n->sigma_l = sigma_l;
    n->lambda = lambda;
    n->c = c;
    n->posThreshold = alpha;
    n->negThreshold = beta;
    n->thresholdMaskBits = TM;
    //n->thresholdPRNMask = getBitMask(n->thresholdMaskBits);
    n->sigmaVR = sigmaVR;
    n->encodedResetVoltage = VR;
    n->resetMode = gamma;
    n->kappa = kappa;
    n->omega = 0;
    


    //! @TODO: perhaps calculate if a neuron is self firing or not.
    n->firedLast = false;
    n->heartbeatOut = false;
    n->isSelfFiring = false;
    n->receivedSynapseMsgs = 0;
    
    setNeuronDest(signalDelay, destGlobalID, n);
    if (n->resetMode == 0) {
        n->doReset = resetLinear;
    }else if (n->resetMode == 1){
        n->doReset = resetNormal;
    }else {
        n->doReset = resetNone;
    }
    
    //synaptic neuron setup:
    n->largestRandomValue = n->thresholdPRNMask;
    if(n->largestRandomValue > 256) {
        tw_error(TW_LOC, 67, "Error - neuron (%i,%i) has a PRN Max greater than 256\n ", n->myCoreID, n->myLocalID);
    }
    //just using this rather than bit shadowing.
    
    n->log = NULL;
    
    //Check to see if we are a self-firing neuron. If so, we need to send heartbeats every big tick.
    n->isSelfFiring = false; //!@TODO: Add logic to support self-firing (spontanious) neurons
    
}

void writeLPState(tw_lp *lp){
    
}


void setNeuronDest(int signalDelay, uint64_t gid, neuronState *n) {

    n->delayVal = signalDelay;
    n->dendriteGlobalDest = gid;
	if(tnMapping != LLINEAR){
    GlobalID g;
    g.raw = gid;
    n->dendriteCore = g.core;
		n->dendriteLocal = g.local; }
	else
		{
			//must be manually set by another init function for now.
		n->dendriteCore = 0;
		n->dendriteLocal = 0;
		}

    
}



/**
 * Neuron message rec. Requires lp access - and sends messages as needed.
 * First, the neuron saves it's current state in the message.
 Next, the neuron checks for the type of message received. If it is a synapse, then it
 integrates the synapse into the current membrane potential. If this is the first synapse
 message the neuron has received and if this neuron is not a spontanious neuron, a heartbeat
 message is sent. The nubmer of synapse messages received is incremented by one.
 
 If the message is a heartbeat message, then the neuron resets the received synapse message
 count. The neuron then does the leak function, and finally checks to see if it should
 fire. If it should, then the neuron sends a new fire message to it's destination axon (at the
 next big tick), increases the SOP count, and runs the reset function.
 For heartbeat messages, the neuron sets the last active time to now.
 
 *
 *
 */
void rtSynapse(neuronState *s, tw_lp *lp) {
    s->membranePotential ++;
    if(s->heartbeatOut == false){
        s->heartbeatOut = true;
        tw_stime time = getNextBigTick(lp, s->myLocalID);
        sendHeartbeat(s, time, lp);
    }
    
}

void rtHeartbeat(neuronState *s, tw_lp *lp) {
    s->heartbeatOut = false;
    s->membranePotential = 0;
    fire(s, lp);
}
bool neuronReceiveMessage(neuronState *st, Msg_Data *m, tw_lp *lp, tw_bf *bf)
{
    
    
    //memcpy(&m->neuronVoltage, &st->membranePotential, sizeof(st->membranePotential));
    m->neuronVoltage = st->membranePotential;
    //memcpy(&m->neuronLastLeakTime, &st->lastLeakTime,sizeof(st->lastLeakTime));
    m->neuronLastLeakTime =st->lastLeakTime;
    //m->neuronRcvMsgs = st->receivedSynapseMsgs;
    m->neuronDrawnRandom = st->drawnRandomNumber;
	m->neuronFireCount = st->fireCount;
    
    bf->c14 = st->heartbeatOut; //C14 indicates the old heartbeat state.

    
    //testing reverse code:
//    if(m->eventType == SYNAPSE_OUT){
//        rtSynapse(st, lp);
//        return;
//    }
//    else
//    {
//        rtHeartbeat(st, lp);
//        return;
//    }

    
    //state management
    bool willFire = false;
    //Next big tick:

    

    //memcpy(m->nm, st, sizeof(neuronState));
    
    //TODO: remove this after testing.
    //m->stateSaveZone = tw_calloc(TW_LOC, "neuron", sizeof(neuronState), 1);
    //memcpy(m->stateSaveZone,st,sizeof(*st));
    
    
    switch (m->eventType)
    {
        case SYNAPSE_OUT:
            st->drawnRandomNumber = tw_rand_integer(lp->rng, 0, st->largestRandomValue);
			integrate(m->axonID, st, lp);
            //next, we will check if a heartbeat message should be sent
            if (st->heartbeatOut == false) {
                tw_stime time = getNextBigTick(lp, st->myLocalID);
                st->heartbeatOut = true;
                bf->c13 = 1; //C13 indicates that the heartbeatout flag has been changed.
                sendHeartbeat(st, time,lp);
                
                //set message flag indicating that the heartbeat msg has been sent
                
                
            }
            
            break;
            
        case NEURON_HEARTBEAT:
            st->heartbeatOut = false;
            //set message flag indicating that the heartbeat msg has been sent
            bf->c13 = 1; //C13 indicates that the heartbeatout flag has been changed.
            
            //Currently operates - leak->fire->(reset)
            st->drawnRandomNumber = tw_rand_integer(lp->rng, 0, st->largestRandomValue);

			numericLeakCalc(st, tw_now(lp));
            //linearLeak( st, tw_now(lp));

            willFire = neuronShouldFire(st, lp);
            if (willFire) {
                fire(st,lp);
                st->fireCount++;
                //TODO: Fix this shit:
                st->membranePotential = 0;
                
            }

            neuronPostIntegrate(st, tw_now(lp), lp, willFire);
            //stats collection
            st->SOPSCount++;
            
            st->lastActiveTime = tw_now(lp);
            
            //do we still have more than the threshold volts left? if so,
            //send a heartbeat out that will ensure the neuron fires again.
            if(neuronShouldFire(st, lp)){
                st->heartbeatOut = true;
                tw_stime time = getNextBigTick(lp, st->myLocalID);
                sendHeartbeat(st, time, lp);
                //set message flag indicating that the heartbeat msg has been sent
                bf->c13 = 1; //C13 indicates that the heartbeatout flag has been changed.
                
            }
            
            if(st->isSelfFiring){
                tw_stime time = getNextBigTick(lp, st->myLocalID);
                sendHeartbeat(st, time, lp);
                st->heartbeatOut = true;
                //set message flag indicating that the heartbeat msg has been sent
                bf->c13 = 1; //C13 indicates that the heartbeatout flag has been changed.
            }
            break;
        default:
            //Error condition - non-valid input.
            tw_error(TW_LOC, "Neuron (%i,%i) received invalid message type, %i \n ", st->myCoreID,st->myLocalID, m->eventType);
            break;
    }
    //TODO: Remove this
    
        
    return willFire;
}



void neuronReverseState(neuronState *s, tw_bf *CV, Msg_Data *m, tw_lp *lp)
{

    
    /** @todo - check this for correctness and switch from delta encoding. */
    //TERRIBLE DEBUGGING CODE REMOVE BEFORE ANYONE SEES:
    //memcpy(s, m->stateSaveZone, sizeof(*s));
    
    if (m->eventType == NEURON_HEARTBEAT) {
        s->SOPSCount--;
    }
    
    	if (s->firedLast == true) {
    		s->fireCount--;
    		s->firedLast = false;
    	}
 
    s->membranePotential = m->neuronVoltage;
    s->lastLeakTime = m->neuronLastLeakTime;
    s->lastActiveTime = m->neuronLastActiveTime;
    s->drawnRandomNumber = m->neuronDrawnRandom;
    s->fireCount = m->neuronFireCount;
    
    //check for heartbeat rollback:
    if(CV->c13 == 1){
        s->heartbeatOut = CV->c14;
    }
    
    
}



/** @name LeakFunctions
 * Neuron functions that manage leaks. All voltage state saving
 * must be handled in the neuron event function neuronReceiveMessage().
 * @{
 */
void noLeak(void *neuron, tw_stime now)
{
	// do nothing!!
}


void revNoLeak(void *neuron, tw_stime now)
{
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
void linearLeak(neuronState *neuron, tw_stime now)
{
	neuronState *s = (neuronState *)neuron;
	tw_stime bigTick = getCurrentBigTick(now);

	tw_stime delta = bigTick - s->lastLeakTime;

	//if the leak is probabilistic:
	if (s->c) {
		while (delta >= 1)
		{
			stochasticIntegrate((s->epsilon * s->lambda), s);
			delta--;
		}
	}else    {
		short int omega = (1 - s->epsilon) + s->epsilon *SGN(s->membranePotential);
		s->membranePotential += omega * s->lambda;
	}

	s->lastLeakTime = bigTick;
    
}





void revLinearLeak(void *neuron, tw_stime now)
{
	neuronState *s = (neuronState *)neuron;
	//tw_stime delta = s->lastLeakTime - now;
	///s->membranePot += s->leakRate * delta;
	s->lastLeakTime = now;
}


/** @}*/

/** @name ResetFunctions
 * Reset function defs. Neuron reset functions will
 * change the neuron state without saving the previous state. All voltage state saving
 * must be handled in the neuron event function neuronReceiveMessage().
 * @todo: Check that reverse reset functions are needed, since previous voltage is stored in the neuron.
 * @{ */
void resetNormal(void *neuronState)
{
	struct NeuronModel *s = (struct NeuronModel *)neuronState;

	s->membranePotential = s->resetVoltage; // set current voltage to \f$R\f$.
}


/**
 *  @details Linear reset mode - ignores \f$R\f$, and sets the membrane potential
 *  to the difference between the threshold and the potential.
 *
 */
void resetLinear(void *neuronState)
{
	struct NeuronModel *s = (struct NeuronModel *)neuronState;

	s->membranePotential = s->membranePotential - s->resetVoltage;
}


/// @todo This might not be needed, since we are saving voltage state.
/// \brief reverseResetLinear
/// \param neuronState
///
void reverseResetLinear(void *neuronState)
{
	struct NeuronModel *s = (struct NeuronModel *)neuronState;
    volt_type resetParam = (volt_type)s->resetVoltage;

	s->membranePotential = s->membranePotential - resetParam;
}


/** @todo: Check that this is the proper way to handle reset zero function */
void reverseResetNormal(void *neuronState)
{
}


void resetNone(void *neuronState)
{

}


void reverseResetNone(void *neuronState)
{
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



void integrate(id_type synapseID, neuronState *st, void *lp){
    //tw_lp *l = (tw_lp *) lp;
    weight_type weight = st->synapticWeight[st->axonTypes[synapseID]] & st->synapticConnectivity[synapseID];
    
    if(weight > 0)
        printf("WEIGHT LARGER THAN ZERO!");
    
    
    //!!!! DEBUG CHECK FOR WEIGHT ISSUES:
    weight = 0;
    //!!! REMOVE previous FOR PRODUCTION
    
    
    
    if(st->weightSelection[ st->axonTypes[synapseID]]){ //zero if this is normal, else
        
        stochasticIntegrate(weight, st);
    }
    else
    {
        st->membranePotential += weight;
    }
}

void sendHeartbeat(neuronState *st, tw_stime time, void *lp) {
    tw_lp *l = (tw_lp *) lp;
    tw_event *newEvent = tw_event_new(l->gid, getNextBigTick(l, st->myLocalID), l);
    Msg_Data *data = (Msg_Data *)tw_event_data(newEvent);
    data->localID = st->myLocalID;
    data->eventType=NEURON_HEARTBEAT;
    tw_event_send(newEvent);
    if(st->heartbeatOut == false) {
        tw_error(TW_LOC, 455, "Error - neuron sent heartbeat without setting HB to true\n");
    }

}
bool neuronShouldFire(neuronState *st, void *lp)
{
    
	//check negative threshold values:
    volt_type threshold = st->posThreshold;
    return (st->membranePotential >= threshold);// + (st->drawnRandomNumber));
}



void fire(neuronState *st, void *l)
{
    tw_lp * lp = (tw_lp *) l;
    tw_stime nextHeartbeat = getNextBigTick(lp,st->myLocalID);
	tw_event *newEvent = tw_event_new(st->dendriteGlobalDest, nextHeartbeat, lp);
	Msg_Data *data = (Msg_Data *)tw_event_data(newEvent);

	data->eventType = NEURON_OUT;
	data->localID = st->myLocalID;
	tw_event_send(newEvent);
	st->firedLast = true;
}




/**
 * @details From the Cassidy 2013 paper, page 4, Part D. This function covers the
 * stochastic synaptic and leak integration functions defined in the paper.
 * @param weight weight - can be considered as both \f$
 * @param st
 * @param lp
 */
void stochasticIntegrate(weight_type weight, neuronState *st)
{
    //long drawnRandom = st->drawnRandomNumber;
    //long randV = st->drawnRandomNumber & st->thresholdPRNMask;
	/**@TODO Enable threshold PRN masking! */
    //long rv = st->drawnRandomNumber;
    if (BINCOMP(weight, st->drawnRandomNumber)) {
		st->membranePotential += 1;
	}
}


//non huge array method of integration.
//void integrate(_idT axonID, neuronState *st, tw_lp *lp)
//{
//	_voltT adjWt = st->axonWeightProb[st->weightSelect[axonID]];
//
//	if (st->axonProbSelect[st->weightSelect[axonID]]) {
//		stochasticIntegrate(adjWt, st);
//	} else {
//		st->membranePot += adjWt;
//	}
//}


/** @todo There is potentially an issue here - does the TrueNorth architecture draw a new
 *	pseudorandom number during the threshold, fire, reset functions, or does it resuse them? It
 *	looks like re-use, so that's what I'm doing here.
 */
void neuronPostIntegrate(neuronState *st, tw_stime time, tw_lp *lp, bool willFire)
{
    
	if (willFire) { // neuron will/did fire:
        st->doReset(st);
	} else if (st->membranePotential < -1 * (st->negThreshold * st->resetVoltage + (st->negThreshold + st->drawnRandomNumber))) {
		//sanity variables for the formulaic reset/bounce instead of calling functions:
		thresh_type B = st->negThreshold;
		long long K = st->resetVoltage;
		long long G = st->resetMode;
		rand_type n = st->drawnRandomNumber;
		volt_type R = st->resetVoltage;
		volt_type V = st->membranePotential;
		st->membranePotential = (-(B*K) + (-(DT(G)) * R +
		    DT(G-1) * (V + (B + n)) +
		    DT(G-2) * V) * (1-K));
	}
}



void numericLeakCalc(neuronState *st, tw_stime now) {
    
    //calculate current time since last leak --- LEAK IS TERRIBLE FOR THIS:
    uint_fast32_t numberOfBigTicksSinceLastLeak = getCurrentBigTick(now) - getCurrentBigTick(st->lastLeakTime);
    //then run the leak function until we've caught up:
    for(;numberOfBigTicksSinceLastLeak > 0; numberOfBigTicksSinceLastLeak --) {
        uint64_t omega = st->sigma_l * (1 - st->epsilon) + SGN(st->membranePotential)*st->sigma_l * st->epsilon;
        st->membranePotential = st->membranePotential +
                                    (omega * ((1 - st->c) * st->lambda)) +
                                     (st->c * BINCOMP(st->lambda, st->drawnRandomNumber));
    }
    st->lastLeakTime = now;
}

/** @} */
//void sendHeartbeat(neuronState *st, tw_lp *lp, tw_stime time)
//{
//	//random fn call state management.
//	//printf("heartbeat sent \n");
//
//	tw_stime nextHeartbeat = getNextBigTick(lp, st->myLocalID);
//	tw_event *newEvent = tw_event_new(lp->gid, nextHeartbeat, lp);
//	Msg_Data *data = (Msg_Data *)tw_event_data(newEvent);
//
//	data->eventType = NEURON_HEARTBEAT;
//	data->localID = st->myLocalID;
//
//	tw_event_send(newEvent);
//}

//void neuronReceiveMessage(neuronState *st, tw_stime time, Msg_Data *m, tw_lp *lp)
//{
//	//state management
//	bool willFire = false;
//
//	st->firedLast = false;
//	//save the previous state of the neuron:
//	//st->savedLastLeakTime = st->lastLeakTime;
//	//st->savedLastActiveTime = st->lastActiveTime;
//	//st->savedMembranePot = st->membranePot;
//	m->neuronVoltage = st->membranePot;
//	m->neuronLastLeakTime = st->lastLeakTime;
//	m->neuronLastActiveTime = st->lastActiveTime;
//
//
//
//	switch (m->eventType)
//	{
//	case SYNAPSE_OUT:
//		integrateSynapseFast(m->axonID, st, lp);
//
//		//next, we will check if a heartbeat message should be sent
//		if (st->receivedSynapseMsgs == 0) {
//			sendHeartbeat(st, lp, time);
//		}
//		st->receivedSynapseMsgs++;
//
//
//		break;
//
//	case NEURON_HEARTBEAT:
//		st->receivedSynapseMsgs = 0;
//		//set up drawn random number for the heartbeat.
//		//if(st->thresholdPRNMask != 0)
//		st->drawnRandomNumber = tw_rand_integer(lp->rng, 0, INT32_MAX) & st->thresholdPRNMask;
//
//		//Currently operates - leak->fire->(reset)
//
//		st->doLeak(st, time);
//		willFire = neuronShouldFire(st, lp);
//		if (willFire) {
//			nSpike(st, time, lp);
//			st->fireCount++;
//		}
//
//		neuronPostIntegrate(st, time, lp, willFire);
//		//stats collection
//		st->SOPSCount++;
//		st->lastActiveTime = tw_now(lp);
//		break;
//	default:
//		//Error condition - non-valid input.
//		break;
//	}
//
//	//store the random calls in the message:
//
//
//	st->rcvdMsgCount++;
//}

