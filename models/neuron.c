//
//  neuron.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#include "neuron.h"


/** Constructor / Init a new neuron. assumes that the reset voltage is NOT encoded (i.e.,
  * a reset value of -5 is allowed. Sets reset voltage sign from input reset voltage).*/
void initNeuron(id_type coreID, id_type nID,
                bool synapticConnectivity[NEURONS_IN_CORE],
                short G_i[NEURONS_IN_CORE], short sigma[4],
                short S[4], bool b[4], bool epsilon,
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
    n->sigmaVR = SGN(VR);
    n->encodedResetVoltage = VR;
    n->resetVoltage = VR; //* sigmaVR;
    
    n->resetMode = gamma;
    n->kappa = kappa;
    n->omega = 0;

    //! @TODO: perhaps calculate if a neuron is self firing or not.
    n->firedLast = false;
    n->heartbeatOut = false;
    //n->isSelfFiring = false;
    //n->receivedSynapseMsgs = 0;

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
        tw_error(TW_LOC, "Error - neuron (%i,%i) has a PRN Max greater than 256\n ", n->myCoreID, n->myLocalID);
    }
    //just using this rather than bit shadowing.

    n->log = NULL;
    n->dendriteLocal = destAxonID;
    n->dendriteGlobalDest = destGlobalID;

    //Check to see if we are a self-firing neuron. If so, we need to send heartbeats every big tick.
    //n->isSelfFiring = false; //!@TODO: Add logic to support self-firing (spontanious) neurons

}
void initNeuronEncodedRV(id_type coreID, id_type nID,
                         bool synapticConnectivity[NEURONS_IN_CORE],
                         short G_i[NEURONS_IN_CORE], short sigma[4],
                         short S[4], bool b[4], bool epsilon,
                         short sigma_l, short lambda, bool c, uint32_t alpha,
                         uint32_t beta, short TM, short VR, short sigmaVR, short gamma,
                         bool kappa, neuronState *n, int signalDelay, uint64_t destGlobalID,int destAxonID) {
    
    initNeuron(coreID, nID, synapticConnectivity, G_i,  sigma, S, b, epsilon,
               sigma_l, lambda, c, alpha, beta,  TM,  VR,  sigmaVR,  gamma,
                kappa,  n,  signalDelay, destGlobalID, destAxonID);
    n->sigmaVR = sigmaVR;
    n->encodedResetVoltage = VR;
    n->resetVoltage = (n->sigmaVR * (pow(2, n->encodedResetVoltage)-1));
    
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

		n->dendriteCore = lGetCoreFromGID(gid);
		n->dendriteLocal = lGetAxeNumLocal(gid);
		}


}


/**
 *  neuronReceiveMessage():
 *  @details  neruonReceiveMessage runs when an event is received by a neuron.
 *  The events are handled in the following fashion:
 *  - If the event is a synapse, the neuron integrates the weight of the synapse into its membrane potential.
 *    - The count of big-tick synapse messages recived is incremented.
 *    - If the count changes from zero to one, the function creates and queues a heartbeat event for the next big-tick.
 *  - If the event is a heartbeat, the neuron will leak, potentially fire, and reset.
 *  - self-firing neurons are not properly implemented, as are neurons that leak down without bound.
 *    -Will be added later though...
 *
 *  Big-Tick events are sent at time \f$𝒯_{bigTick} + ε\f$, so that the the event will arrive at the next bit-tick time.
 * @todo remove delta encoding
 *
 */



bool neuronReceiveMessage(neuronState *st, Msg_Data *m, tw_lp *lp, tw_bf *bf)
{


    //memcpy(&m->neuronVoltage, &st->membranePotential, sizeof(st->membranePotential));
    m->neuronVoltage = st->membranePotential;
    //memcpy(&m->neuronLastLeakTime, &st->lastLeakTime,sizeof(st->lastLeakTime));
    m->neuronLastLeakTime =st->lastLeakTime;
    //m->neuronRcvMsgs = st->receivedSynapseMsgs;
    m->neuronDrawnRandom = st->drawnRandomNumber;
    //m->neuronFireCount = st->fireCount;

    bf->c14 = st->heartbeatOut; //C14 indicates the old heartbeat state.


    //state management
    bool willFire = false;
    //Next big tick:



    //memcpy(m->nm, st, sizeof(neuronState));

    ///@TODO: remove this after testing.
    //m->stateSaveZone = tw_calloc(TW_LOC, "neuron", sizeof(neuronState), 1);
    //memcpy(m->stateSaveZone,st,sizeof(*st));

    int num = st->myLocalID;

    switch (m->eventType)
    {
      /// @TODO: possibly need to aggregate inputs on the same channel? If validation isn't working check this.
            

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
            ringing(st, m->neuronVoltage);
            
            //willFire = neuronShouldFire(st, lp); //removed and replaced with fireFloorCelingReset
            willFire = fireFloorCelingReset(st, lp);
            
            if (willFire) {
                fire(st,lp);
                //st->fireCount++;
                //TODO: Fix this shit:
					//st->membranePotential = 0;
            }

            //neuronPostIntegrate(st, tw_now(lp), lp, willFire); //removed and replaced with fireFloorCelingReset
            //stats collection
            st->SOPSCount++;
            st->lastActiveTime = tw_now(lp);

            
//            if(neuronShouldFire(st, lp)){
//                st->heartbeatOut = true;
//                tw_stime time = getNextBigTick(lp, st->myLocalID);
//                sendHeartbeat(st, time, lp);
//                //set message flag indicating that the heartbeat msg has been sent
//                bf->c13 = 1; //C13 indicates that the heartbeatout flag has been changed.
//
//
//            }
            //do we still have more than the threshold volts left? if so,
            //send a heartbeat out that will ensure the neuron fires again.
            //Or if we are as self-firing neuron.
            ///@TODO: Add detection of self-firing neuron state.
            if( neuronShouldFire(st, lp) && st->heartbeatOut == false ){
                tw_stime time = getNextBigTick(lp, st->myLocalID);
                st->heartbeatOut = true;
                //set message flag indicating that the heartbeat msg has been sent
                bf->c13 = 1; //C13 indicates that the heartbeatout flag has been changed.
                sendHeartbeat(st, time, lp);
            }
            break;
        default:
            //Error condition - non-valid input.
            tw_error(TW_LOC, "Neuron (%i,%i) received invalid message type, %i \n ", st->myCoreID,st->myLocalID, m->eventType);
            break;
    }
    //self-firing neuron (spont.)
    if(st->isSelfFiring && st->heartbeatOut == false){
        tw_stime time = getNextBigTick(lp, st->myLocalID);
        st->heartbeatOut = true;
        //set message flag indicating that the heartbeat msg has been sent
        bf->c13 = 1; //C13 indicates that the heartbeatout flag has been changed.
        sendHeartbeat(st, time, lp);
    }
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
    		//s->fireCount--;
    		s->firedLast = false;
    	}

    s->membranePotential = m->neuronVoltage;
    s->lastLeakTime = m->neuronLastLeakTime;
    s->lastActiveTime = m->neuronLastActiveTime;
    s->drawnRandomNumber = m->neuronDrawnRandom;
    //s->fireCount = m->neuronFireCount;

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
 * @details
 * Reset function defs. Neuron reset functions will
 * change the neuron state without saving the previous state. All voltage state saving
 * must be handled in the neuron event function neuronReceiveMessage().
 * These functions operate based on the table presented in \cite Cass13_1000 .
 * Currently, there are three functions, one for linear reset (resetLinear()),
 * "normal" reset (resetNormal()), and non-reset (resetNone()).
 
 From the paper:
 | \f$𝛾_j\f$ | \f$𝜘_j\f$| Reset Mode               |     Positive Reset     |     Negative Reset    |
 |----|----|--------------------------|:----------------------:|:---------------------:|
 | 0  | 0  | normal                   |          \f$R_j\f$         |         \f$-R_j\f$        |
 | 0  | 1  | normal - neg saturation  |          \f$R_j\f$         |       \f$-𝛽_j\f$        |
 | 1  | 0  | Linear                   | \f$V_j - (𝛼_j  + 𝜂_j)\f$ | \f$V_j + (𝛽_j + 𝜂_j)\f$ |
 | 1  | 1  | linear -neg saturation   |  \f$V_j - (𝛼_j,+ 𝜂_j)\f$ |        \f$-𝛽_j\f$        |
 | 2  | 0  | non-reset                |          \f$V_j\f$         |         \f$V_j\f$         |
 | 2  | 1  | non-reset net saturation |          \f$V_j\f$         |        \f$-𝛽_j\f$        |
 
 * @todo: Check that reverse reset functions are needed, since previous voltage is stored in the neuron.
 * @{ */

/**
 * negative saturation reset function (common to all reset modes, called if
 * 𝛾 is true. Simply sets the value of the membrane potential to $-𝛽_j$.
**/
void negThresholdReset(neuronState *s) {
    s->membranePotential = -s->negThreshold;
}
/**

 *  @details Normal reset function.
 */
void resetNormal(void *neuronState)
{
	struct NeuronModel *s = (struct NeuronModel *)neuronState;
    if(s->membranePotential < s->negThreshold){
        if(s->kappa)
            negThresholdReset(s);
        else
            s->membranePotential = -(s->resetVoltage);
    }
    else {
        s->membranePotential = s->resetVoltage; // set current voltage to \f$R\f$.
    }
}


/**
 *  @details Linear reset mode - ignores \f$R\f$, and sets the membrane potential
 *  to the difference between the threshold and the potential. *
 */
void resetLinear(void *neuronState)
{
	struct NeuronModel *s = (struct NeuronModel *)neuronState;

    if(s->membranePotential < s->negThreshold){
        if(s->kappa)
            negThresholdReset(s);
        else{
            s->membranePotential = s->membranePotential -
                (s->negThreshold+ s->drawnRandomNumber);
        }
    }else{
        s->membranePotential = s->membranePotential -
            (s->posThreshold  + s->drawnRandomNumber);
    }
        
}
/**
 *  @details non-reset handler function - does non-reset style reset. Interestingly,
 *  even non-reset functions follow the negative saturation parameter from the paper. 
 */
void resetNone(void *neuronState)
{
    struct NeuronModel *s = (struct NeuronModel *)neuronState;

    if(s->kappa && s->membranePotential < s->negThreshold){
        negThresholdReset(s);
    }
    
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





void reverseResetNone(void *neuronState)
{
}

/** From Neuron Behavior Reference - checks to make sure that there is no "ringing".
 The specs state that the leak stores the voltage in a temporary variable. Here,
 we store the leak voltage in the membrane potential, and override it with a new value. */

void ringing(void *nsv, volt_type oldVoltage ){
    neuronState *ns = (neuronState *) nsv;
    if(ns->epsilon && (SGN(ns->membranePotential) != SGN(oldVoltage))){
        ns->membranePotential = 0;
    }
}

/** @} */


/** @name NeuronFunctions
 * Main neuron functions and behaviors
 *  @{
 */





void integrate(id_type synapseID, neuronState *st, void *lp){
    //tw_lp *l = (tw_lp *) lp;
	int at = st->axonTypes[synapseID];
	weight_type stw = st->synapticWeight[at];
		//weight_type weight = st->synapticWeight[st->axonTypes[synapseID]] & st->synapticConnectivity[synapseID];
	weight_type weight = stw *  st->synapticConnectivity[synapseID];
    //!!!! DEBUG CHECK FOR WEIGHT ISSUES:
    //weight = 0;
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
    //tw_event *newEvent = tw_event_new(l->gid, (0.1 + (tw_rand_unif(l->rng) / 1000)),l);
    Msg_Data *data = (Msg_Data *)tw_event_data(newEvent);
    data->localID = st->myLocalID;
    data->eventType=NEURON_HEARTBEAT;
    tw_event_send(newEvent);
    if(st->heartbeatOut == false) {
        tw_error(TW_LOC, "Error - neuron sent heartbeat without setting HB to true\n");
    }

}

bool overUnderflowCheck(void *ns){
    neuronState *n = (neuronState *) ns;
    
    int ceiling = 393216;
    int floor = -393216;
    bool spike = false;
    if(n->membranePotential > ceiling){
        spike = true;
        n->membranePotential = ceiling;
    }else if(n->membranePotential < floor){
        n->membranePotential = floor;
    }
    return spike;
}
bool neuronShouldFire(neuronState *st, void *lp)
{

	//check negative threshold values:
    volt_type threshold = st->posThreshold;
    return (st->membranePotential >= threshold);// + (st->drawnRandomNumber));
}
/**
* Changes neuron state - only use when actually attempting to fire, do not use
* to check if neuron should fire - use neuronShouldFire() for that.
* Calls underflow/overflow check first, then checks voltage.
* Then numerically calculates positive and negative reset voltages, and
* sets them in the neuron state. Internally handles PRN draw and "masking".
 */
bool fireFloorCelingReset(neuronState *ns, tw_lp *lp){
    bool shouldFire = false;
     ///@TODO remove this line later for performacne - check that neuron init handles this properly.
    ns->drawnRandomNumber = 0;
    //Sanity variables - remove if we need that .01% performance increase:
    volt_type Vrst = ns->resetVoltage;
    volt_type alpha = ns->posThreshold;
    volt_type beta = ns->negThreshold;
    short gamma = ns->resetMode;
    
    
    ///@TODO: might not need this random generator, if it is called in the event handler here
    if(ns->c)
        ns->drawnRandomNumber = (tw_rand_integer(lp->rng, 0, ns->largestRandomValue));
    //check if neuron is overflowing/underflowing:
    if(overUnderflowCheck((void*)ns)) {
        return true;
    }else if(ns->membranePotential >= ns->posThreshold + ns->drawnRandomNumber){
        ns->membranePotential = ((DT(gamma))*Vrst +
                                ((DT((gamma-1))) * (ns->membranePotential - (alpha + ns->drawnRandomNumber))) +
                                ((DT((gamma-2))) * ns->membranePotential)
                                 );
        
//        
//        switch (ns->resetMode) {
//            case 0:
//                ns->membranePotential = ns->resetVoltage;
//                break;
//            case 1:
//                ns->membranePotential = DT(𝛾)  *
//                (ns->membranePotential - (ns->posThreshold + ns->drawnRandomNumber));
//                break;
//            case 2:
//                //ns->membranePotential = DT * ns->membranePotential;
//                break;
//            default:
//                tw_error(TW_LOC, "Error occured within neuron reset function - neuron had invalid reset mode %i", ns->resetMode);
//                break;
        
        shouldFire = true;
    }else if (ns->membranePotential <
              (-1 * (beta * ns->kappa +
                    (beta + ns->drawnRandomNumber) * (1 - ns->kappa)
                     ))){
        volt_type x = ns->membranePotential;
        //x = ((-1 * beta) * ns->kappa);
        volt_type s1,s2,s3,s4;
        s1 = (-1*beta) * ns->kappa;
        s2 = (-1*(DT(gamma))) * Vrst;
        s3 = (DT((gamma - 1)) * (ns->membranePotential + (beta + ns->drawnRandomNumber)));
        s4 = (DT((gamma - 2)) * ns->membranePotential) * (1 - ns->kappa);
        x = s1 + (s2 + s3 + s4);
        
        ns->membranePotential = (
                                 ((-1*beta) * ns->kappa) + (
                                 ((-1*(DT(gamma))) * Vrst) +
                                 ((DT((gamma - 1))) * (ns->membranePotential + (beta + ns->drawnRandomNumber))) +
                                 ((DT((gamma - 2))) * ns->membranePotential)) * (1 - ns->kappa)
                                 );
                                                    
    }
    return shouldFire;
    
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
	} else if (st->membranePotential < -1 * (st->negThreshold * st->resetVoltage +
            (st->negThreshold + st->drawnRandomNumber))) {
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


/** NOTE: Assumes that if this neuron's leak parameters will create spontanious
  * spiking, then this function will be called for every big-tick during the simulation.
  *
  */
void numericLeakCalc(neuronState *st, tw_stime now) {
    //shortcut for calcuation - neurons do not leak if:
    //lambda is zero:
    if(st->lambda == 0)
        return;
    //calculate current time since last leak --- LEAK IS TERRIBLE FOR THIS:
    uint_fast32_t numberOfBigTicksSinceLastLeak = getCurrentBigTick(now) - getCurrentBigTick(st->lastLeakTime);
    //then run the leak function until we've caught up:
    for(;numberOfBigTicksSinceLastLeak > 0; numberOfBigTicksSinceLastLeak --) {
        uint64_t omega = st->sigma_l * (1 - st->epsilon) + SGN(st->membranePotential)*st->sigma_l * st->epsilon;
        st->membranePotential = st->membranePotential +
                                    (omega * ((1 - st->c) * st->lambda)) +
                                     (st->c & (BINCOMP(st->lambda, st->drawnRandomNumber)));

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
