//
// Created by Mark Plagge on 5/25/16.
//

#include "tn_neuron.h"

/** \defgroup TN_Function_hdrs True North Function headers
 * TrueNorth Neuron leak, integrate, and fire function forward decs.
 * @{ */

void TNFire(tn_neuron_state *st, void *l);

/**
 *  @brief  function that adds a synapse's value to the current neuron's
 * membrane potential.
 *
 *  @param synapseID localID of the synapse sending the message.
 */
void TNIntegrate(id_type synapseID, tn_neuron_state *st, void *lp);
/**
*  @brief  handles incomming synapse messages. In this model, the neurons send
messages to axons during "big tick" intervals.
This is done through an event sent upon receipt of the first synapse message of
the current big-tick.
*
*  @param st   current neuron state
*  @param m    event message data
*  @param lp   lp.
*/

bool TNReceiveMessage(tn_neuron_state *st, messageData *M, tw_lp *lp,
					  tw_bf *bf);

/**
 * @brief handels reverse computation and state messages.
 * @param st current neuron state
 * @param M reverse message
 * @param lp the lp
 * @param bf the reverse computation bitfield.
 */

void TNReceiveReverseMessage(tn_neuron_state *st, messageData *M, tw_lp *lp,
							 tw_bf *bf);

/**
 *  @brief  Checks to see if a neuron should fire.
 *  @todo check to see if this is needed, since it looks like just a simple if
 * statement is in order.
 *
 *  @param st neuron state
 *
 *  @return true if the neuron is ready to fire.
 */
bool TNShouldFire(tn_neuron_state *st, tw_lp *lp);

/**
 * @brief New firing system using underflow/overflow and reset.
 * @return true if neuron is ready to fire. Membrane potential is set
 * regardless.
 */
bool TNfireFloorCelingReset(tn_neuron_state *ns, tw_lp *lp);

/**
 *  @brief  Neuron stochastic integration function - for use with stochastic
 * leaks and synapse messages.
 *
 *  @param weight weight of selected leak or synapse
 *  @param st     the neuron state
 */
void TNstochasticIntegrate(weight_type weight, tn_neuron_state *st);

/**
 *  @brief NumericLeakCalc - uses formula from the TrueNorth paper to calculate
 * leak.
 *  @details Will run $n$ times, where $n$ is the number of big-ticks that have
 * occured since
 *  the last integrate. Handles stochastic and regular integrations.
 *
 *  @TODO: self-firing neurons will not properly send messages currently - if
 * the leak is divergent, the flag needs to be set upon neuron init.
 *  @TODO: does not take into consideration thresholds. Positive thresholds
 * would fall under self-firing neurons, but negative thresholds should be
 * reset.
 *  @TODO: test leaking functions
 */
void TNNumericLeakCalc(tn_neuron_state *st, tw_stime now);

void TNLinearLeak(tn_neuron_state *neuron, tw_stime now);

void TNSendHeartbeat(tn_neuron_state *st, tw_stime time, void *lp);

/**
 *  @brief  Function that runs after integration & firing, for reset function
 * and threshold bounce calls.
 *
 *  @param st      state
 *  @param time    event time
 *  @param lp      lp
 *  @param didFire did the neuron fire during this big tick?
 */
void TNPostIntegrate(tn_neuron_state *st, tw_stime time, tw_lp *lp,
					 bool didFire);

/** From Neuron Behavior Reference - checks to make sure that there is no
 "ringing".
 The specs state that the leak stores the voltage in a temporary variable. Here,
 we store the leak voltage in the membrane potential, and override it with a new
 value. */

void ringing(void *nsv, volt_type oldVoltage);

/** @} */

/**
 * \ingroup TN_RESET True North Reset
 * True North Leak, Integrate and Fire Functions
 * Reset function defs. Neuron reset functions will
 * change the neuron state without saving the previous state. All voltage state
 saving
 * must be handled in the neuron event function neuronReceiveMessage().
 * These functions operate based on the table presented in \cite Cass13_1000 .
 * Currently, there are three functions, one for linear reset (resetLinear()),
 * "normal" reset (resetNormal()), and non-reset (resetNone()).

 From the paper:
 | \f$𝛾_j\f$ | \f$𝜘_j\f$| Reset Mode               |     Positive Reset     |
 Negative Reset    |
 |----|----|--------------------------|:----------------------:|:---------------------:|
 | 0  | 0  | normal                   |          \f$R_j\f$         |
 \f$-R_j\f$        |
 | 0  | 1  | normal - neg saturation  |          \f$R_j\f$         |
 \f$-𝛽_j\f$        |
 | 1  | 0  | Linear                   | \f$V_j - (𝛼_j  + 𝜂_j)\f$ | \f$V_j + (𝛽_j
 + 𝜂_j)\f$ |
 | 1  | 1  | linear -neg saturation   |  \f$V_j - (𝛼_j,+ 𝜂_j)\f$ |
 \f$-𝛽_j\f$        |
 | 2  | 0  | non-reset                |          \f$V_j\f$         |
 \f$V_j\f$         |
 | 2  | 1  | non-reset net saturation |          \f$V_j\f$         |
 \f$-𝛽_j\f$        |

 * @todo: Check that reverse reset functions are needed, since previous voltage
 is stored in the neuron.
 * @{ */

/**
 * negative saturation reset function (common to all reset modes, called if
 * 𝛾 is true. Simply sets the value of the membrane potential to $-𝛽_j$.
**/
void negThresholdReset(tn_neuron_state *s) {
	s->membranePotential = -s->negThreshold;
}

/**

 * Normal reset function.
 */
void resetNormal(void *neuronState) {
	tn_neuron_state *s = (tn_neuron_state *) neuronState;
	if (s->membranePotential < s->negThreshold) {
		if (s->kappa)
			negThresholdReset(s);
		else
			s->membranePotential = -(s->resetVoltage);
	} else {
		s->membranePotential = s->resetVoltage;  // set current voltage to \f$R\f$.
	}
}

/**
 *   Linear reset mode - ignores \f$R\f$, and sets the membrane potential
 *  to the difference between the threshold and the potential. *
 */
void resetLinear(void *neuronState) {
	tn_neuron_state *s = (tn_neuron_state *) neuronState;

	if (s->membranePotential < s->negThreshold) {
		if (s->kappa)
			negThresholdReset(s);
		else {
			s->membranePotential =
					s->membranePotential - (s->negThreshold + s->drawnRandomNumber);
		}
	} else {
		s->membranePotential =
				s->membranePotential - (s->posThreshold + s->drawnRandomNumber);
	}
}

/**
 *   non-reset handler function - does non-reset style reset. Interestingly,
 *  even non-reset functions follow the negative saturation parameter from the
 * paper.
 */
void resetNone(void *neuronState) {
	tn_neuron_state *s = (tn_neuron_state *) neuronState;

	if (s->kappa && s->membranePotential < s->negThreshold) {
		negThresholdReset(s);
	}
}

/**@} */
/** @defgroup TN_fire_reset True North Reset Fire
*   True North reset and fire functions
* @{ */

/** From Neuron Behavior Reference - checks to make sure that there is no
 "ringing".
 The specs state that the leak stores the voltage in a temporary variable. Here,
 we store the leak voltage in the membrane potential, and override it with a new
 value. */
void TNFire(tn_neuron_state *st, void *l) {
	tw_lp *lp = (tw_lp *) l;
	// DEBUG
	//	tw_lpid outid = st->dendriteGlobalDest;
	//	tw_lp *destLP = tw_getlp(outid);
	//	printf("Sending message to %llu\n", destLP->gid);

	// DEBUG
	tw_stime nextHeartbeat = getNextBigTick(lp, st->myLocalID);
	tw_event *newEvent = tw_event_new(st->outputGID, nextHeartbeat, lp);
	messageData *data = (messageData *) tw_event_data(newEvent);

	data->eventType = NEURON_OUT;
	data->localID = st->myLocalID;

    if (isDestInterchip(st->myCoreID, getCoreFromGID(st->outputGID))) {
        data->isRemote = st->myCoreID;

    }
	tw_event_send(newEvent);
	st->firedLast = true;
}

bool TNReceiveMessage(tn_neuron_state *st, messageData *m, tw_lp *lp,
					  tw_bf *bf) {
	/** @todo Replace these state saving values with reverse computation. */
	m->neuronVoltage = st->membranePotential;
	m->neuronLastLeakTime = st->lastLeakTime;
	m->neuronDrawnRandom = st->drawnRandomNumber;
	// m->neuronFireCount = st->fireCount;

	// bf->c14 = st->heartbeatOut; //C14 indicates the old heartbeat state.
	// state management
	bool willFire = false;
	// Next big tick:

	//@todo: see if this is used still and remove
	// int num = st->myLocalID;

	switch (m->eventType) {
		/// @TODO: possibly need to aggregate inputs on the same channel? If
		/// validation isn't working check this.

		case SYNAPSE_OUT:
			st->drawnRandomNumber = tw_rand_integer(
					lp->rng, 0,
					st->largestRandomValue);  //!<- @BUG This might be creating
			//! non-deterministic errors
			TNIntegrate(m->axonID, st, lp);
			// next, we will check if a heartbeat message should be sent
			if (st->heartbeatOut == false) {
				tw_stime time = getNextBigTick(lp, st->myLocalID);
				st->heartbeatOut = true;
				bf->c13 =
						1;  // C13 indicates that the heartbeatout flag has been changed.
				TNSendHeartbeat(st, time, lp);

				// set message flag indicating that the heartbeat msg has been sent
			}
			break;

		case NEURON_HEARTBEAT:
			st->heartbeatOut = false;
			// set message flag indicating that the heartbeat msg has been sent
			bf->c13 =
					1;  // C13 indicates that the heartbeatout flag has been changed.

			// Currently operates - leak->fire->(reset)
			st->drawnRandomNumber =
					tw_rand_integer(lp->rng, 0, st->largestRandomValue);

			TNNumericLeakCalc(st, tw_now(lp));
			// linearLeak( st, tw_now(lp));
			ringing(st, m->neuronVoltage);

			// willFire = neuronShouldFire(st, lp); //removed and replaced with
			// fireFloorCelingReset
			willFire = TNfireFloorCelingReset(st, lp);
			bf->c0 = willFire;

			if (willFire) {
				TNFire(st, lp);
				//check for intra-core communications -
				//setting bit 31 as toggle for send communication
				if (isDestInterchip(st->myCoreID, getCoreFromGID(st->outputGID))) {
					bf->c31 = 1;
				} else {
					bf->c31 = 0;
				}

				// st->fireCount++;
			}

			st->lastActiveTime = tw_now(lp);


			// do we still have more than the threshold volts left? if so,
			// send a heartbeat out that will ensure the neuron fires again.
			// Or if we are as self-firing neuron.
			///@TODO: Add detection of self-firing neuron state.
			///@TODO: Ensure bf-c13 state validity here for reverse computations
			if (TNShouldFire(st, lp) && st->heartbeatOut == false) {
				tw_stime time = getNextBigTick(lp, st->myLocalID);
				st->heartbeatOut = true;
				// set message flag indicating that the heartbeat msg has been sent
				bf->c13 =
						1;  // C13 indicates that the heartbeatout flag has been changed.
				TNSendHeartbeat(st, time, lp);
			}
			break;
		default:
			// Error condition - non-valid input.
			tw_error(TW_LOC, "Neuron (%i,%i) received invalid message type, %i \n ",
					 st->myCoreID, st->myLocalID, m->eventType);
			break;
	}
	// self-firing neuron (spont.)
	if (st->isSelfFiring && st->heartbeatOut == false) {
		tw_stime time = getNextBigTick(lp, st->myLocalID);
		st->heartbeatOut = true;
		// set message flag indicating that the heartbeat msg has been sent
		bf->c13 = 1;  // C13 indicates that the heartbeatout flag has been changed.
		TNSendHeartbeat(st, time, lp);
	}
	return willFire;
}

void TNReceiveReverseMessage(tn_neuron_state *st, messageData *M, tw_lp *lp,
							 tw_bf *bf) {
	if (M->eventType == NEURON_HEARTBEAT) {
		// reverse heartbeat message
		// st->SOPSCount--;
	}
	if (bf->c0) {  // c0 flags firing state
		// reverse computation of fire and reset functions here.
		/**@todo implement neuron fire/reset reverse computation functions */
	}
	if (bf->c13) {
		st->heartbeatOut = !st->heartbeatOut;
	}
	/**@todo remove this once neuron reverse computation functions are built. */
	st->membranePotential = M->neuronVoltage;
	st->lastLeakTime = M->neuronLastLeakTime;
	st->lastActiveTime = M->neuronLastActiveTime;
	st->drawnRandomNumber = M->neuronDrawnRandom;
}

/**
 * @brief      From Neuron Behavior Reference - checks to make sure that there
 is no "ringing".
 The specs state that the leak stores the voltage in a temporary variable. Here,
 we store the leak voltage in the membrane potential, and override it with a new
 value.
 *
 * @param      nsv         The neuron state
 * @param[in]  oldVoltage  The old voltage
 */
void ringing(void *nsv, volt_type oldVoltage) {
	tn_neuron_state *ns = (tn_neuron_state *) nsv;
	if (ns->epsilon && (SGN(ns->membranePotential) != SGN(oldVoltage))) {
		ns->membranePotential = 0;
	}
}

void TNIntegrate(id_type synapseID, tn_neuron_state *st, void *lp) {
	// tw_lp *l = (tw_lp *) lp;
	// int at = st->axonTypes[synapseID];
	bool con = st->synapticConnectivity[synapseID];
	// DEBUG CODE REMOVE FOR PRODUCTION:
	// id_type myid = st->myLocalID;

	if (con == 0) return;
	// printf("id-%llu, sid-%llu, connect: %i\n",myid, synapseID,con);
	// weight_type stw = st->synapticWeight[at];
	weight_type weight = st->synapticWeight[st->axonTypes[synapseID]] &&
						 st->synapticConnectivity[synapseID];
	//!!!! DEBUG CHECK FOR WEIGHT ISSUES:
	// weight = 0;
	//!!! REMOVE previous FOR PRODUCTION

	if (st->weightSelection[st->axonTypes[synapseID]]) {  // zero if this is
		// normal, else

		TNstochasticIntegrate(weight, st);
	} else {
		st->membranePotential += weight;
	}
}

void TNSendHeartbeat(tn_neuron_state *st, tw_stime time, void *lp) {
	tw_lp *l = (tw_lp *) lp;
	tw_event *newEvent =
			tw_event_new(l->gid, getNextBigTick(l, st->myLocalID), l);
	// tw_event *newEvent = tw_event_new(l->gid, (0.1 + (tw_rand_unif(l->rng) /
	// 1000)),l);
	messageData *data = (messageData *) tw_event_data(newEvent);
	data->localID = st->myLocalID;
	data->eventType = NEURON_HEARTBEAT;
	tw_event_send(newEvent);
	if (st->heartbeatOut == false) {
		tw_error(TW_LOC,
				 "Error - neuron sent heartbeat without setting HB to true\n");
	}
}

bool overUnderflowCheck(void *ns) {
	tn_neuron_state *n = (tn_neuron_state *) ns;

	int ceiling = 393216;
	int floor = -393216;
	bool spike = false;
	if (n->membranePotential > ceiling) {
		spike = true;
		n->membranePotential = ceiling;
	} else if (n->membranePotential < floor) {
		n->membranePotential = floor;
	}
	return spike;
}

bool TNShouldFire(tn_neuron_state *st, tw_lp *lp) {
	// check negative threshold values:
	volt_type threshold = st->posThreshold;
	return (st->membranePotential >= threshold);  // + (st->drawnRandomNumber));
}

bool TNfireFloorCelingReset(tn_neuron_state *ns, tw_lp *lp) {
	bool shouldFire = false;
	///@TODO remove this line later for performacne - check that neuron init
	/// handles this properly.
	ns->drawnRandomNumber = 0;
	// Sanity variables - remove if we need that .01% performance increase:
	volt_type Vrst = ns->resetVoltage;
	volt_type alpha = ns->posThreshold;
	volt_type beta = ns->negThreshold;
	// DEBUG DELTA GAMMA
	//	short deltaGamma = DT(ns->resetMode);
	//	short dg1 = DT(ns->resetMode -1);
	//	short dg2 = DT(ns->resetMode -2);
	//	short tg2 = !(ns->resetMode -2);
	short gamma = ns->resetMode;

	///@TODO: might not need this random generator, if it is called in the event
	/// handler here
	if (ns->c)
		ns->drawnRandomNumber =
				(tw_rand_integer(lp->rng, 0, ns->largestRandomValue));
	// check if neuron is overflowing/underflowing:
	if (overUnderflowCheck((void *) ns)) {
		return true;
	} else if (ns->membranePotential >=
			   ns->posThreshold + ns->drawnRandomNumber) {
		// reset:
		ns->membranePotential =
				((DT(gamma)) * Vrst) +
				((DT(gamma - 1)) * (Vj - (alpha + ns->drawnRandomNumber))) +
				((DT(gamma - 2)) * Vj);
		// volt_type mp = ns->membranePotential;
		shouldFire = true;
	} else if (ns->membranePotential <
			   (-1 * (beta * ns->kappa +
					  (beta + ns->drawnRandomNumber) * (1 - ns->kappa)))) {
		// volt_type x = ns->membranePotential;
		// x = ((-1 * beta) * ns->kappa);
		//        volt_type s1,s2,s3,s4;
		//        s1 = (-1*beta) * ns->kappa;
		//        s2 = (-1*(DT(gamma))) * Vrst;
		//        s3 = (DT((gamma - 1)) * (ns->membranePotential + (beta +
		//        ns->drawnRandomNumber)));
		//        s4 = (DT((gamma - 2)) * ns->membranePotential) * (1 - ns->kappa);
		//		//x = s1 + (s2 + s3 + s4);

		ns->membranePotential =
				(((-1 * beta) * ns->kappa) +
				 (((-1 * (DT(gamma))) * Vrst) +
				  ((DT((gamma - 1))) *
				   (ns->membranePotential + (beta + ns->drawnRandomNumber))) +
				  ((DT((gamma - 2))) * ns->membranePotential)) *
				 (1 - ns->kappa));
	}
	return shouldFire;
}

void TNstochasticIntegrate(weight_type weight, tn_neuron_state *st) {
	if (BINCOMP(weight, st->drawnRandomNumber)) {
		st->membranePotential += 1;
	}
}

void TNPostIntegrate(tn_neuron_state *st, tw_stime time, tw_lp *lp,
					 bool willFire) {
	if (willFire) {  // neuron will/did fire:
		// st->doReset(st);
	} else if (st->membranePotential <
			   -1 * (st->negThreshold * st->resetVoltage +
					 (st->negThreshold + st->drawnRandomNumber))) {
		// sanity variables for the formulaic reset/bounce instead of calling
		// functions:
		thresh_type B = st->negThreshold;
		long long K = st->resetVoltage;
		long long G = st->resetMode;
		random_type n = st->drawnRandomNumber;
		volt_type R = st->resetVoltage;
		volt_type V = st->membranePotential;
		st->membranePotential =
				(-(B * K) +
				 (-(DT(G)) * R + DT(G - 1) * (V + (B + n)) + DT(G - 2) * V) * (1 - K));
	}
}

void TNNumericLeakCalc(tn_neuron_state *st, tw_stime now) {
	// shortcut for calcuation - neurons do not leak if:
	// lambda is zero:
	if (st->lambda == 0) return;
	// calculate current time since last leak --- LEAK IS TERRIBLE FOR THIS:
	uint_fast32_t numberOfBigTicksSinceLastLeak =
			getCurrentBigTick(now) - getCurrentBigTick(st->lastLeakTime);
	// then run the leak function until we've caught up:
	for (; numberOfBigTicksSinceLastLeak > 0; numberOfBigTicksSinceLastLeak--) {
		uint64_t omega = st->sigma_l * (1 - st->epsilon) +
						 SGN(st->membranePotential) * st->sigma_l * st->epsilon;

		st->membranePotential =
				st->membranePotential + (omega * ((1 - st->c) * st->lambda)) +
				(st->c & (BINCOMP(st->lambda, st->drawnRandomNumber)));
	}
	st->lastLeakTime = now;
}

/** @} ******************************************************/

/**
 * \defgroup TN_REVERSE TN Reverse
 * True North Reverse Leak, Integrate, and Fire Functions
 * @{
 */

/** @} */

/** \defgroup TNParams TN Parameters
 * TrueNorth Neuron Parameter setting functions. Used as helper functions for
 * init
 * @{ */

void TN_set_neuron_dest(int signalDelay, uint64_t gid, tn_neuron_state *n) {
	n->delayVal = signalDelay;
	n->outputGID = gid;
}

/** @} */

//*********************************************************************************
/** \defgroup TNNeuronInit TrueNorth Init
 *  TrueNorth Neuron initialization functions
 * @{ */
/** Constructor / Init a new neuron. assumes that the reset voltage is NOT
 * encoded (i.e.,
  * a reset value of -5 is allowed. Sets reset voltage sign from input reset
 * voltage).*/
void tn_create_neuron(id_type coreID, id_type nID,
					  bool synapticConnectivity[NEURONS_IN_CORE],
					  short G_i[NEURONS_IN_CORE], short sigma[4], short S[4],
					  bool b[4], bool epsilon, short sigma_l, short lambda,
					  bool c, uint32_t alpha, uint32_t beta, short TM, short VR,
					  short sigmaVR, short gamma, bool kappa,
					  tn_neuron_state *n, int signalDelay,
					  uint64_t destGlobalID, int destAxonID) {
	for (int i = 0; i < 4; i++) {
		n->synapticWeight[i] = sigma[i] * S[i];
		n->weightSelection[i] = b[i];
	}
	for (int i = 0; i < NEURONS_IN_CORE; i++) {
		n->synapticConnectivity[i] = synapticConnectivity[i];
		n->axonTypes[i] = G_i[i];
	}

	// set up other parameters
	n->myCoreID = coreID;
	n->myLocalID = nID;
	n->epsilon = epsilon;
	n->sigma_l = sigma_l;
	n->lambda = lambda;
	n->c = c;
	n->posThreshold = alpha;
	n->negThreshold = beta;
	// n->thresholdMaskBits = TM;
	// n->thresholdPRNMask = getBitMask(n->thresholdMaskBits);
	n->sigmaVR = SGN(VR);
	n->encodedResetVoltage = VR;
	n->resetVoltage = VR;  //* sigmaVR;

	n->resetMode = gamma;
	n->kappa = kappa;
	n->omega = 0;

	//! @TODO: perhaps calculate if a neuron is self firing or not.
	n->firedLast = false;
	n->heartbeatOut = false;
	// n->isSelfFiring = false;
	// n->receivedSynapseMsgs = 0;

	TN_set_neuron_dest(signalDelay, destGlobalID, n);

	// synaptic neuron setup:
	n->largestRandomValue = n->thresholdPRNMask;
	if (n->largestRandomValue > 256) {
		tw_error(TW_LOC, "Error - neuron (%i,%i) has a PRN Max greater than 256\n ",
				 n->myCoreID, n->myLocalID);
	}
	// just using this rather than bit shadowing.

	n->dendriteLocal = destAxonID;
	n->outputGID = destGlobalID;

	// Check to see if we are a self-firing neuron. If so, we need to send
	// heartbeats every big tick.
	n->isSelfFiring =
			false;  //!@TODO: Add logic to support self-firing (spontanious) neurons
}

void tn_create_neuron_encoded_rv(
		id_type coreID, id_type nID, bool synapticConnectivity[NEURONS_IN_CORE],
		short G_i[NEURONS_IN_CORE], short sigma[4], short S[4], bool b[4],
		bool epsilon, short sigma_l, short lambda, bool c, uint32_t alpha,
		uint32_t beta, short TM, short VR, short sigmaVR, short gamma, bool kappa,
		tn_neuron_state *n, int signalDelay, uint64_t destGlobalID,
		int destAxonID) {
	tn_create_neuron(coreID, nID, synapticConnectivity, G_i, sigma, S, b, epsilon,
					 sigma_l, lambda, c, alpha, beta, TM, VR, sigmaVR, gamma,
					 kappa, n, signalDelay, destGlobalID, destAxonID);
	n->sigmaVR = sigmaVR;
	n->encodedResetVoltage = VR;
	n->resetVoltage = (n->sigmaVR * (pow(2, n->encodedResetVoltage) - 1));
}

/**
 * @brief      Creates a simple neuron for a identity matrix benchmark.
 *  Weights are set up such that axon $n$ has weight 1, where $n$ is the
 *  neuron local id. Other axons have weight 0. Leak is set to zero as well.
 *  The output axon is a randomly selected axon.
 *
 *
 * @param      s     { parameter_description }
 * @param      lp    The pointer to a
 */
void TN_create_simple_neuron(tn_neuron_state *s, tw_lp *lp) {
	// Rewrote this function to have a series of variables that are easier to
	// read.
	// Since init time is not so important, readability wins here.
	// AutoGenerated test neurons:
	static int created = 0;
	bool synapticConnectivity[NEURONS_IN_CORE];
	short G_i[NEURONS_IN_CORE];
	short sigma[4];
	short S[4] = {[0] = 3};
	bool b[4];
	bool epsilon = 0;
	bool sigma_l = 0;
	short lambda = 0;
	bool c = false;
	short TM = 0;
	short VR = 0;
	short sigmaVR = 1;
	short gamma = 0;
	bool kappa = 0;
	int signalDelay = 1;  // tw_rand_integer(lp->rng, 0,5);

	for (int i = 0; i < NEURONS_IN_CORE; i++) {
		// s->synapticConnectivity[i] = tw_rand_integer(lp->rng, 0, 1);
		s->axonTypes[i] = 1;
		G_i[i] = 0;
		synapticConnectivity[i] = 0;
		// synapticConnectivity[i] = tw_rand_integer(lp->rng, 0, 1)
	}

	id_type myLocalID = getNeuronLocalFromGID(lp->gid);

	synapticConnectivity[myLocalID] = 1;

	//(creates an "ident. matrix" of neurons.
	for (int i = 0; i < 4; i++) {
		// int ri = tw_rand_integer(lp->rng, -1, 0);
		// unsigned int mk = tw_rand_integer(lp->rng, 0, 1);

		// sigma[i] = (!ri * 1) + (-1 & ri))
		// sigma[i] = (mk ^ (mk - 1)) * 1;
		sigma[i] = 1;
		b[i] = 0;
	}

	// weight_type alpha = tw_rand_integer(lp->rng, THRESHOLD_MIN, THRESHOLD_MAX);
	// weight_type beta = tw_rand_integer(lp->rng, (NEG_THRESH_SIGN *
	// NEG_THRESHOLD_MIN), NEG_THRESHOLD_MAX);
	weight_type alpha = 1;
	weight_type beta = -1;
	// DEBUG LINE

	tn_create_neuron_encoded_rv(
			getCoreFromGID(lp->gid), getNeuronLocalFromGID(lp->gid),
			synapticConnectivity, G_i, sigma, S, b, epsilon, sigma_l, lambda, c,
			alpha, beta, TM, VR, sigmaVR, gamma, kappa, s, signalDelay, 0, 0);
	// we re-define the destination axons here, rather than use the constructor.

	float remoteCoreProbability = .905;
	long int dendriteCore = s->myCoreID;
	// This neuron's core is X. There is a 90.5% chance that my destination will
	// be X - and a 10% chance it will be a different core.
	if (tw_rand_unif(lp->rng) < remoteCoreProbability) {
		//		long dendriteCore = s->myCoreID;
		//		dendriteCore = tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);
		dendriteCore = tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);
	}

	/**@note This random setup will create neurons that have an even chance of
   * getting an axon inside thier own core
   * vs an external core. The paper actually capped this value at something like
   * 20%. @todo - make this match the
   * paper if performance is slow. * */
	// s->dendriteCore = tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);
	s->dendriteLocal =
			s->myLocalID;  // tw_rand_integer(lp->rng, 0, AXONS_IN_CORE - 1);
	//     if (tnMapping == LLINEAR) {
	s->outputGID = getAxonGlobal(dendriteCore, s->dendriteLocal);
	created++;
}

/** @} */

/** /defgroup TN_ROSS_HANDLERS
 * Implementations of TN functions that are called by ross. Forward, reverse,
 * init, etc.
 * @{
 * */

void TN_init(tn_neuron_state *s, tw_lp *lp) {
	static int fileInit = 0;
	///// DUMPI FILE
	if (!fileInit) {
		char *fn = calloc(sizeof(char), 256);
		sprintf(fn, "dumpi_virt-%i_rnk%li.txt", getCoreFromGID(lp->gid), g_tw_mynode);
		dumpi_out = fopen(fn, "w");
		free(fn);
		fileInit = 1;
	}

	static int pairedNeurons = 0;
	static bool announced = false;
	s->neuronTypeDesc = "SIMPLE";
	if (DEBUG_MODE && !announced) {
		printf("Creating neurons\n");
		announced = true;
	}
	// ADD FILE INPUT NEURON CREATION HERE

	TN_create_simple_neuron(s, lp);

	// createDisconnectedNeuron(s, lp);

	//    messageData *setupMsg;
	//    tw_event *setupEvent;
	//
	//    setupEvent = tw_event_new(lp->gid, getNextEventTime(lp), lp);
	//    setupMsg = (messageData *) tw_event_data(setupEvent);
	//
	//
	//    bool * connectivity = tw_calloc(TW_LOC,"LP",sizeof(bool),AXONS_IN_CORE);
	//    for (int i = 0; i < AXONS_IN_CORE; i ++){
	//        connectivity[i] = s->synapticWeight[s->axonTypes[i]];
	//    }
	//    setupMsg->eventType = NEURON_SETUP;
	//    setupMsg->localID = s->myLocalID;
	//    setupMsg->neuronConn = connectivity;
	//
	//    tw_event_send(setupEvent);

	if (DEBUG_MODE) {
		printf(
				"Neuron type %s, num: %llu checking in with GID %llu and dest %llu \n",
				s->neuronTypeDesc, s->myLocalID, lp->gid, s->outputGID);
	}
	// Original NeMo Neuron Init
	//    static int pairedNeurons = 0;
	//    s->neuronTypeDesc = "SIMPLE";
	//    if(DEBUG_MODE && ! annouced)
	//        printf("Creating neurons\n");
	//
	//    if(PHAS_VAL) {
	//        if(!pc){
	//            crPhasic(s, lp);
	//            pc = true;
	//        }
	//        else {
	//            createDisconnectedNeuron(s, lp);
	//        }
	//
	//    } else if(TONIC_BURST_VAL) {
	//        if(pairedNeurons < 2) {
	//            crTonicBursting(s, lp);
	//            pairedNeurons ++;
	//        }
	//        else {
	//            createDisconnectedNeuron(s, lp);
	//        }
	//    } else if (PHASIC_BURST_VAL){
	//        if (pairedNeurons < 2) {
	//            crPhasicBursting(s, lp);
	//            pairedNeurons ++;
	//        }
	//
	//    } else {
	//        createSimpleNeuron(s, lp);
	//    }
	//    //createDisconnectedNeuron(s, lp);
	//    annouced = true;
}

void TN_forward_event(tn_neuron_state *s, tw_bf *CV, messageData *m,
					  tw_lp *lp) {
	long start_count = lp->rng->count;

	//    //Delta Encoding
	//    if (g_tw_synchronization_protocol == OPTIMISTIC ||
	//        g_tw_synchronization_protocol == OPTIMISTIC_REALTIME ||
	//        g_tw_synchronization_protocol == OPTIMISTIC_DEBUG) {
	//        // Only do this in OPTIMISTIC mode
	//        tw_snapshot(lp, lp->type->state_sz);
	//    }

	if (VALIDATION || SAVE_MEMBRANE_POTS) {  // If we are running model validation
		// or we are saving membrane
		// potentials

		// saveNeruonState(s->myLocalID, s->myCoreID, s->membranePotential,
		// tw_now(lp));
	}

	bool fired = TNReceiveMessage(s, m, lp, CV);
	s->SOPSCount++;
	/**@todo save message trace here: */

	CV->c0 = fired;  // save fired information for reverse computation.

	if (fired &&
		(SAVE_SPIKE_EVTS ||
		 VALIDATION)) {  // if we are validating the model or saving spike
		// events, save this event's info.

		// write_event(s->myLocalID, s->myCoreID, s->s->outputGID, 'N', tw_now(lp));
	}
	m->rndCallCount = lp->rng->count - start_count;

	//    tw_snapshot_delta(lp, lp->type->state_sz);
}

void TN_reverse_event(tn_neuron_state *s, tw_bf *CV, messageData *m,
					  tw_lp *lp) {
	long count = m->rndCallCount;
	//    tw_snapshot_restore(lp, lp->type->state_sz);
	if (VALIDATION || SAVE_MEMBRANE_POTS) {
		// reverse save neuron state;
	}

	TNReceiveReverseMessage(s, m, lp, CV);
	s->SOPSCount--;
	if (CV->c0 && (SAVE_SPIKE_EVTS || VALIDATION)) {
		// reverse_write_event
	}

	while (count--) tw_rand_reverse_unif(lp->rng);
}

/** TN_commit is a function called on commit. This is used for management of
 * neurons! */
void TN_commit(tn_neuron_state *s, tw_bf *cv, messageData *m, tw_lp *lp) {
	// if neuron has fired and save neuron fire events is enabled, save this
	// event.
	if (SAVE_SPIKE_EVTS && cv->c0) {
		saveNeuronFire(tw_now(lp), s->myCoreID, s->myLocalID, s->outputGID);
	}

	// save simulated dumpi trace if inter core and dumpi trace is on
	/** @TODO: Add dumpi save flag to config. */
	if (cv->c31) {
        //saveMPIMessage(s->myCoreID, getCoreFromGID(s->outputGID), tw_now(lp),
        //			   dumpi_out);

        saveSendMessage(s->myCoreID, getCoreFromGID(s->outputGID), tw_now(lp), dumpi_out);

    }


}

/** @todo: fix this remote value */
void prhdr(bool *display, char *hdr) {
	/*
	if (&display) {
	  //print(hdr);
	  *display = true;
	}
	 */
}

void TN_final(tn_neuron_state *s, tw_lp *lp) {
	static int fileOpen = 1;

	if (fileOpen) {
		fclose(dumpi_out);
		fileOpen = 0;
	}
	if (g_tw_synchronization_protocol == OPTIMISTIC_DEBUG) {
		// Alpha, SOPS should be zero. HeartbeatOut should be false.
		char *em = (char *) calloc(1024, sizeof(char));
		char *hdr = "------ Neuron Optimistic Debug Check -----";
		char *alpha = "--->Membrane Potential is: ";
		char *sops = "--->SOPS is:";
		char *HB = "--->Heartbeat is:";
		bool dsp = false;

		sprintf(em, "%s\n Core: %i Local: %i \n", hdr, s->myCoreID, s->myLocalID);
		if (s->membranePotential != 0) {
			prhdr(&dsp, em);
			//debugMsg(alpha, s->membranePotential);
		}
		if (s->SOPSCount != 0) {
			prhdr(&dsp, em);
			//debugMsg(sops, s->SOPSCount);
		}
		if (s->heartbeatOut != false) {
			prhdr(&dsp, em);

		}
	}
}

inline tn_neuron_state *TN_convert(void *lpstate) {
	return (tn_neuron_state *) lpstate;
}

/*@}*/
