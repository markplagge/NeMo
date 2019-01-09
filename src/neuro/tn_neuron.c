//
// Created by Mark Plagge on 5/25/16.
//

#include "tn_neuron.h"

/** Testing Values @{*/
#ifdef NET_IO_DEBUG
//#include "../tests/nemo_tests.h"
char *BBFN = "neuron_config_test";
FILE *neuronConfigFile;
char configFileName[128];

#endif
/*@}*/

/** Enum for parsing CSV data */
enum arrayFtr {
  CONN, //Syn. Connectivity
  AXTP, //Axon Types
  SGI, //sigma GI vals
  SP, //S Vals
  BV, //b vals
  NEXT, //goto next array data chunk
  OUT //out of array data
};
//nextToI() is a quick way to reduce the amount I type the ATOI function.
//In Globals.h, there is a proto-macro that I'm working on that will
//auto-choose the right string conversion function.

//TODO: move this to a sane location or possibly replace with nice functions.
#define nextToI()    atoi(raw.rawDatM[currentFld++]);

//File output handle.
FILE *dumpi_out;
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

bool TNReceiveMessage(tn_neuron_state *st, messageData *m, tw_lp *lp,
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


bool fireTimingCheck(tn_neuron_state *s, tw_lp *lp) {
  //check to see if the fire event should occur.
  //Neurons can output one spike per big tick.
  //in Debug mode, we set the last fired time and update the count.
  //If we are in the next big tick then everything is okay.
  tw_stime now = tw_now(lp);
  unsigned long currentBigTick = getCurrentBigTick(now);
  unsigned long prevBigTick = getCurrentBigTick(s->lastFire);

  if (currentBigTick==0) { //big tick is zero so we are at first
    s->firecount++;
    return true;
  }
  //check to see if we are on a new tick:
  if (currentBigTick > prevBigTick) {
    // we are in a new big tick. This is okay.
    s->firecount = 1;
    s->lastFire = now;
    return true;
  } else if (currentBigTick==prevBigTick) { //fire request on the same tick - error condition 1
    s->firecount++;

//#ifdef DEBUG
//    tw_error(TW_LOC, "Neuron fire rate error. Current big tick: %lu \t FireCount: %i. Neuron Core: %i Local: %i ",
//             currentBigTick, s->firecount, s->myCoreID, s->myLocalID);
//#endif
    return false;
  } else {
    //Unknown error state - BigTick < prevBigTick.
    //tw_error(TW_LOC, "Out of order big ticks! PrevActive: %lu, CurrentTick: %lu", prevBigTick, currentBigTick);
    //Reverse computation dude.
    return true;
  }

}


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

  //tw_lp *lp = (tw_lp *) l;


  // DEBUG
  //	tw_lpid outid = st->dendriteGlobalDest;
  //	tw_lp *destLP = tw_getlp(outid);
  //	printf("Sending message to %llu\n", destLP->gid);


  //} else {
  tw_lp *lp = (tw_lp *) l;
  // DEBUG
  //	tw_lp *destLP = tw_getlp(outid);
  //	tw_lpid outid = st->dendriteGlobalDest;
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
  //}
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
    if (st->heartbeatOut==false) {
      tw_stime time = getNextBigTick(lp, st->myLocalID);
      st->heartbeatOut = true;
      bf->c13 =
          1;  // C13 indicates that the heartbeatout flag has been changed.
      TNSendHeartbeat(st, time, lp);

      // set message flag indicating that the heartbeat msg has been sent
    }
    break;

  case NEURON_HEARTBEAT: st->heartbeatOut = false;
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
    willFire = (willFire && fireTimingCheck(st, lp));
    bf->c0 = bf->c0 || willFire;

    if (willFire) {
      if (!st->isOutputNeuron) {
        TNFire(st, lp);
        //check for intra-core communications -
        //setting bit 31 as toggle for send communication




        // st->fireCount++;
      } else {
        /** bf->c10 is an output neuron fire state checker. True means the neuron fired this turn. */
        bf->c10 = 1;

      }
        if (isDestInterchip(st->myCoreID, getCoreFromGID(st->outputGID))) {
            bf->c31 = 1;
            //m->dumpiID = tw_rand_ulong(lp->rng,0,ULONG_MAX - 1);
        } else {
            bf->c31 = 0;
        }
    }

    st->lastActiveTime = tw_now(lp);


    // do we still have more than the threshold volts left? if so,
    // send a heartbeat out that will ensure the neuron fires again.
    // Or if we are as self-firing neuron.
    ///@TODO: Add detection of self-firing neuron state.
    ///@TODO: Ensure bf-c13 state validity here for reverse computations
    if (TNShouldFire(st, lp) && st->heartbeatOut==false) {
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
#ifdef DEBUG
tw_printf(TW_LOC, "Invalid message type received. Dumping information..."
                  "\n message source: %li "
                  "\n message o gid %lu:\n"
                  "---",
                  m->localID, m->originGID);
#endif
    tw_error(TW_LOC, "Neuron (%i,%i) received invalid message type, %i \n ",
             st->myCoreID, st->myLocalID, m->eventType);

    break;
  }
  // self-firing neuron (spont.)
  if (st->isSelfFiring && st->heartbeatOut==false) {
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
  if (M->eventType==NEURON_HEARTBEAT) {
    // reverse heartbeat message
    st->SOPSCount--;
  }
  if (bf->c0) {  // c0 flags firing state
    // reverse computation of fire and reset functions here.
    /**@todo implement neuron fire/reset reverse computation functions */
    st->firedLast = false;

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
  if (ns->epsilon && (SGN(ns->membranePotential)!=SGN(oldVoltage))) {
    ns->membranePotential = 0;
  }
}

void TNIntegrate(id_type synapseID, tn_neuron_state *st, void *lp) {
  // tw_lp *l = (tw_lp *) lp;
  // int at = st->axonTypes[synapseID];
  bool con = st->synapticConnectivity[synapseID];
  // DEBUG CODE REMOVE FOR PRODUCTION:
  // id_type myid = st->myLocalID;

  if (con==0)
    return;
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
  if (st->heartbeatOut==false) {
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
  return (st->membranePotential >= threshold && fireTimingCheck(st, lp));  // + (st->drawnRandomNumber));
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
        ((DT(gamma))*Vrst) +
            ((DT(gamma - 1))*(Vj - (alpha + ns->drawnRandomNumber))) +
            ((DT(gamma - 2))*Vj);
    // volt_type mp = ns->membranePotential;
    shouldFire = true;
  } else if (ns->membranePotential <
      (-1*(beta*ns->kappa +
          (beta + ns->drawnRandomNumber)*(1 - ns->kappa)))) {
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
        (((-1*beta)*ns->kappa) +
            (((-1*(DT(gamma)))*Vrst) +
                ((DT((gamma - 1)))*
                    (ns->membranePotential + (beta + ns->drawnRandomNumber))) +
                ((DT((gamma - 2)))*ns->membranePotential))*
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
      -1*(st->negThreshold*st->resetVoltage +
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
        (-(B*K) +
            (-(DT(G))*R + DT(G - 1)*(V + (B + n)) + DT(G - 2)*V)*(1 - K));
  }
}

void TNNumericLeakCalc(tn_neuron_state *st, tw_stime now) {
  // shortcut for calcuation - neurons do not leak if:
  // lambda is zero:
  if (st->lambda==0)
    return;
  // calculate current time since last leak --- LEAK IS TERRIBLE FOR THIS:
  uint_fast32_t numberOfBigTicksSinceLastLeak =
      getCurrentBigTick(now) - getCurrentBigTick(st->lastLeakTime);
  // then run the leak function until we've caught up:
  volt_type newMP = st->membranePotential;
  short lamb = st->lambda;
  short drawnRandom = st->drawnRandomNumber;
  short c = st->c;
  int64_t omega = st->sigma_l*(1 - st->epsilon) +
      SGN(st->membranePotential)*st->sigma_l*st->epsilon;
  for (; numberOfBigTicksSinceLastLeak > 0; numberOfBigTicksSinceLastLeak--) {
//    int64_t omega = st->sigma_l * (1 - st->epsilon) +
//        SGN(st->membranePotential) * st->sigma_l * st->epsilon;

//    st->membranePotential =
//        st->membranePotential + (omega * ((1 - st->c) * st->lambda)) +
//            (st->c & (BINCOMP(st->lambda, st->drawnRandomNumber)));
    //st->membranePotential =
    newMP += (omega*((1 - st->c)*lamb)) +
        (c & (BINCOMP(lamb, drawnRandom)));
  }
  st->membranePotential = newMP;
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
    n->synapticWeight[i] = sigma[i]*S[i];
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
void tn_create_neuron_encoded_rv_non_global(
                                        int coreID, int nID, bool synapticConnectivity[NEURONS_IN_CORE],
                                        short G_i[NEURONS_IN_CORE], short sigma[4], short S[4], bool b[4],
                                        bool epsilon, int sigma_l, int lambda, bool c, int alpha,
                                        int beta, int TM, int VR, int sigmaVR, int gamma, bool kappa,
                                        tn_neuron_state *n, int signalDelay, int destCoreID,
                                       int destAxonID){
    uint64_t dest_global = getNeuronGlobal(destCoreID, destAxonID);
    tn_create_neuron_encoded_rv(coreID, nID, synapticConnectivity, G_i, sigma, S, b, epsilon, sigma_l, lambda, c, alpha, beta, TM, VR, sigmaVR, gamma, kappa, n, signalDelay, dest_global, destAxonID);
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
  n->resetVoltage = (n->sigmaVR*(pow(2, n->encodedResetVoltage) - 1));
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
  int signalDelay = 0;  // tw_rand_integer(lp->rng, 0,5);

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
/**
 * Details - must be called from TN_init after TN_create_simple_neuron() has been called.
 * @param s
 * @param lp
 */
void TN_create_saturation_neuron(tn_neuron_state *s, tw_lp *lp) {

  static uint64_t numCreated = 0;
  bool synapticConnectivity[NEURONS_IN_CORE];
  getSynapticConnectivity(synapticConnectivity, lp);
  for (int i = 0; i < NEURONS_IN_CORE; i++) {
    // s->synapticConnectivity[i] = tw_rand_integer(lp->rng, 0, 1);
    s->axonTypes[i] = 1;
    s->synapticConnectivity[i] = synapticConnectivity[i];
  }
  for (int i = 0; i < NUM_NEURON_WEIGHTS; i++) {
    s->synapticWeight[i] = connectedWeight;
  }
  s->lambda = SAT_NET_LEAK;
  s->c = SAT_NET_STOC;
  s->posThreshold = SAT_NET_THRESH;
  s->negThreshold = (0 - SAT_NET_THRESH);
  numCreated++;
  if (numCreated >= NEURONS_IN_CORE*CORES_IN_SIM) {
    printf("SAT network finished init \n");
    clearBucket();
  }
}


#define LB sprintf(em, "%s\n%s", em,linebreak)

void modelErr(char *element, char *filename, id_type coreID, id_type lID, char *tp, char *fnDat, int codeline) {
  char *em = calloc(4096, sizeof(char));
  char *linebreak = calloc(128, sizeof(char));
  int i = 0;
  for (; i < 100; i++) {
    linebreak[i] = '-';
  }
  linebreak[i + 1] = '\n';
  LB;
  char *emp1 = "Error while loading NeMo Model config file. Possible bad option.\n";
  sprintf(em, "%s", emp1);
  sprintf(em, "%s\n Filename: %s", em, filename);
  char loc[128];
  sprintf(loc, "%i%s%i", coreID, tp, lID);
  sprintf(em, "%s \n NeuronID: %s", em, loc);
  sprintf(em, "%s \n While loading parameter %s ", em, element);
  LB;
  sprintf(em, "%s \n -- Error type: %s \n", em, fnDat);
  LB;
  sprintf(em, "%s%s", em, "Config File Details: \n");
  getModelErrorInfo(coreID, lID, tp, element, 0);
  printf("%s", em);
  tw_error(TW_LOC, em);

}
int safeGetArr(int direct, char *lutName, char *dirName, long vars[],
               int expectedParams, int cid, int lid, char *ntype) {
  char *errtps[4] = {"0 - No Parameters Loaded From File", "1 - Too Few Parameters Loaded From File",
                     "2 - Parameter Not Found - Assuming 0 values.", "3 - Too Many Values Found"};
  int validation = 0;
  char *pn;
  if (direct) {
    validation = lGetAndPushParam(dirName, 1, vars);
    pn = dirName;
  } else {
    validation = lGetAndPushParam(luT(lutName), 1, vars);
    pn = lutName;
  }
  ++validation;
  if (validation==expectedParams) {
    return validation;
  } else if (validation==0) {
    modelErr(pn, NEMO_MODEL_FILE_PATH, cid, lid, ntype, errtps[0], 917);

  } else if (validation < expectedParams && validation > 0) {
    modelErr(pn, NEMO_MODEL_FILE_PATH, cid, lid, ntype, errtps[1], 917);
  } else if (validation > expectedParams) {
    modelErr(pn, NEMO_MODEL_FILE_PATH, cid, lid, ntype, errtps[3], 917);
  } else {
    modelErr(pn, NEMO_MODEL_FILE_PATH, cid, lid, ntype, errtps[2], 917);
  }
  return validation;
}
//**** Some loader helper macros.
//! Debug network
FILE *core_connectivity_map;
//! @TODO: Move these helper macros somewhere better! .
#define LGT(V) ( lGetAndPushParam( luT( (V) ) , 0, NULL ) )
//#define GA(N, T) (getArray( (#N) , &(N), (T) ))
#define TID core, nid, "TN"
static long num_neg_found = 0;
void TNPopulateFromFile(tn_neuron_state *st, tw_lp *lp) {
  static int core_con_open = 0;

  int extraParamCache = 32;
  // Set up neuron - first non array params:
  tw_lpid outputGID = 0;
  long outputCore;
  long outputLID;
  id_type core = getCoreFromGID(lp->gid);
  id_type nid = getNeuronLocalFromGID(lp->gid);

  outputCore = lGetAndPushParam("destCore", 0, NULL);
  outputLID = lGetAndPushParam("destLocal", 0, NULL);
#ifdef DEBUG
  static int found_one = 0;

  if (outputCore < -100) {
    if (found_one==0) {
      tw_printf(TW_LOC, "Found a negative core! Good! %lli", outputCore);
      found_one = 1;
    }
    num_neg_found++;
  }
#endif
  if (outputCore < 0 || outputLID < 0) {
    //st->outputGID = 0;
    st->isOutputNeuron = true;

    st->outputCoreDest = outputCore;
    st->outputNeuronDest = outputLID;

  } else {
    outputGID = getAxonGlobal(outputCore, outputLID);
    st->outputGID = outputGID;
    st->outputCoreDest = outputCore;
    st->outputNeuronDest = outputLID;

  }
  char *v1 = luT("lambda");

  short lambda = LGT("lambda");
  short resetMode = LGT("resetMode");
  volt_type resetVoltage = LGT("resetVoltage");
  short sigmaVR = LGT("sigmaVR");
  short encodedResetVoltage = LGT("encodedResetVoltage");
  short sigma_l = LGT("sigma_l");
  bool isOutputNeuron = LGT("isOutputNeuron");
  bool epsilon = LGT("epsilon");
  bool c = LGT("c");
  bool kappa = LGT("kappa");
  //bool isActiveNeuron = 		LGT("isActiveNeuron");
  bool isActiveNeuron = true;
  volt_type alpha = lGetAndPushParam("alpha", 0, NULL);
  volt_type beta = lGetAndPushParam("beta", 0, NULL);

  short TM = lGetAndPushParam("TM", 0, NULL);
  short VR = lGetAndPushParam("VR", 0, NULL);
  short gamma = lGetAndPushParam("gamma", 0, NULL);

  bool synapticConnectivity[NEURONS_IN_CORE];
  short axonTypes[NEURONS_IN_CORE];
  short sigma[NUM_NEURON_WEIGHTS];
  short S[NUM_NEURON_WEIGHTS];
  bool b[NUM_NEURON_WEIGHTS];

  long vars[NEURONS_IN_CORE + extraParamCache];
  long validation = 0;

  //@TODO: move the string types to a lookup table

  validation = safeGetArr(0, "synapticConnectivity", NULL, vars, NEURONS_IN_CORE, TID);
//	validation = lGetAndPushParam(luT("synapticConnectivity"), 1, vars);
  if (validation > 0 && validation==NEURONS_IN_CORE) {
    for (int i = 0; i < validation; i++)
      synapticConnectivity[i] = vars[i];
  }
  validation = safeGetArr(0, "axonTypes", NULL, vars, NEURONS_IN_CORE, TID);
//	validation = lGetAndPushParam(luT("axonTypes"), 1, vars);
  if (validation > 0 && validation==NEURONS_IN_CORE) {
    for (int i = 0; i < validation; i++)
      axonTypes[i] = vars[i];
  }
  validation = safeGetArr(0, "sigma", NULL, vars, NUM_NEURON_WEIGHTS, TID);
//	validation = lGetAndPushParam(luT("sigma"), 1, vars);
  if (validation > 0 && validation==NUM_NEURON_WEIGHTS) {
    for (int i = 0; i < validation; i++)
      sigma[i] = vars[i];
  }
  validation = safeGetArr(1, NULL, "S", vars, NUM_NEURON_WEIGHTS, TID);
//	validation = lGetAndPushParam("S", 1, vars);
  if (validation > 0 && validation==NUM_NEURON_WEIGHTS) {
    for (int i = 0; i < validation; i++)
      S[i] = vars[i];
  }
  validation = safeGetArr(1, NULL, "b", vars, NUM_NEURON_WEIGHTS, TID);
//	validation = lGetAndPushParam(luT("b"), 1, vars);
  if (validation > 0 && validation==NUM_NEURON_WEIGHTS) {
    for (int i = 0; i < validation; i++)
      b[i] = vars[i];
  }
  tn_create_neuron_encoded_rv(core,
                              nid,
                              synapticConnectivity,
                              axonTypes,
                              sigma,
                              S,
                              b,
                              epsilon,
                              sigma_l,
                              lambda,
                              c,
                              alpha,
                              beta,
                              TM,
                              VR,
                              sigmaVR,
                              gamma,
                              kappa,
                              st,
                              0,
                              outputGID,
                              outputLID);
  st->resetMode = resetMode;
  st->isActiveNeuron = isActiveNeuron;
  st->isOutputNeuron = isOutputNeuron;
//    if(g_tw_mynode == 0){
//    	if(st->myLocalID==NEURONS_IN_CORE - 1 || st->myLocalID == 0){
//		printf("Completed loading neurons in core %i", st->myCoreID);
//	}
//    }
  if (outputGID >= SIM_SIZE) {
    // printf("err cond 1.\n");
    //st->isActiveNeuron = false;
  }

  clearNeuron(core, nid);
  clearStack();
}


/** @} */
/** attempts to find this neuron's def. from the file input.
 Files are assumed to be inited during main().

 
 */
void TNCreateFromFile(tn_neuron_state *s, tw_lp *lp) {

  static int needannounce = 1;

  //first, get our Neuron IDs:
  id_type core = getCoreFromGID(lp->gid);
  id_type nid = getNeuronLocalFromGID(lp->gid);
  s->myCoreID = core;
  s->myLocalID = nid;
  char *nt = "TN";

  //If we are using JSON, call the JSON loader lib instead of the LUA stack
  if(NEMO_MODEL_IS_TN_JSON){
    loadNeuronFromJSON(core,nid,s);
    if(s->outputCoreDest < 0 ){
      s->outputGID = 0;
      s->isOutputNeuron = 1;

    }else{
      s->outputGID = getGIDFromLocalIDs(core,nid);
    }
  }else if(NEMO_MODEL_IS_BINARY){
    //load the binary file info.
    bool found = loadNeuronFromBIN(core,nid,s);
    if(!found){
      s->isActiveNeuron = false;
    }


  }else{
    int nNotFound = lookupAndPrimeNeuron(core, nid, nt);
    if (DBG_MODEL_MSGS) {
      printf("Found status: %i \n ", nNotFound);
    }

    if (nNotFound) {
      s->isActiveNeuron = false;

    } else {
      TNPopulateFromFile(s, lp);
    }
  }

  if (needannounce && (g_tw_mynode==0) && s->isOutputNeuron) {
    printf("output neuron created.\n");
    printf("Neuron CORE: %lli - LID: %lli - Dest Core: %li  Local: %li \n",
           s->myCoreID,
           s->myLocalID,
           s->outputCoreDest,
           s->outputNeuronDest);
    needannounce = 0;
  }

}

void TN_pre_run(tn_neuron_state *s, tw_lp *me) {


  static int clean = 0;
  static int core_con_open = 0;

    /** @todo: remove this once debugging connections is done
      //DUMB CSV DEBUG
       */
    debug_neuron_connections(s,me);
    if(!clean){
        debug_init_neuron_json();
    }
    debug_add_neuron_to_json(s,me);
/////////////////////////

    if (!clean) {
#ifdef DEBUG
    tw_printf(TW_LOC, "Lua cleanup from neuron.\n");
    tw_printf(TW_LOC, " Found %lli debug cores.\n", num_neg_found);
#endif
    closeLua();
    clean = 1;
  }
//  if (SAVE_NETWORK_STRUCTURE) {
//    saveNeuronPreRun();
//  }
  if (SAVE_NETWORK_STRUCTURE) {
    if (core_con_open==0) {
#ifdef DEBUG
      tw_printf(TW_LOC, "Core Connectivity Map Init.\n");
      //saveNetworkStructureMPI();
#endif
      char *fn = calloc(128, sizeof(char));
      sprintf(fn, "core_con_r%li.csv", g_tw_mynode);
      core_connectivity_map = fopen(fn, "w");
      free(fn);
      core_con_open = 1;
      if (g_tw_mynode==0) {
        fprintf(core_connectivity_map, "from_core,to_core\n");
      }

    }

    fprintf(core_connectivity_map, "%llu,%llu\n", s->myCoreID, getCoreFromGID(s->outputGID));
    saveNeuronPreRun();

  }
}

/** /defgroup TN_ROSS_HANDLERS
 * Implementations of TN functions that are called by ross. Forward, reverse,
 * init, etc.
 * @{
 * */

void TN_init(tn_neuron_state *s, tw_lp *lp) {
#ifdef DEBUG
  static an = 0;
  if (!an && g_tw_mynode == 0 ) {
    if (SAVE_NETWORK_STRUCTURE) {
      tw_printf(TW_LOC, "SAVING NETWORK STRUCTURE\n");
    } else {
      tw_printf(TW_LOC, "NOT SAVING NETWORK STRUCTURE\n");
    }
    an = 1;
  }

#endif
  static u_int8_t fileInit = 0;
  if (DO_DUMPI) {
    if (!fileInit) {
      char *fn = calloc(sizeof(char), 256);
      sprintf(fn, "dumpi_virt-%i_rnk%li.txt", getCoreFromGID(lp->gid), g_tw_mynode);
      dumpi_out = fopen(fn, "w");
      free(fn);
      fileInit = 1;
    }
  }
  if (fileInit==0 || fileInit==1) {
    printf("Starting tn network init\n");
    fileInit = 3;
  }
  static int pairedNeurons = 0;
  static bool announced = false;
  //s->neuronTypeDesc = "SIMPLE";
  if (DEBUG_MODE && !announced) {
    printf("Creating neurons\n");
    announced = true;
  }
  if (FILE_IN) {
    TNCreateFromFile(s, lp);

  } else {
    TN_create_simple_neuron(s, lp);
    //This if statement should not be needed, due to check in setupGrud.
    if ((LAYER_NET_MODE & GRID_LAYER) || (LAYER_NET_MODE & CONVOLUTIONAL_LAYER)) {
      configureNeuronInLayer(s, lp);
    } else if (IS_SAT_NET) {
    }
    TN_create_saturation_neuron(s, lp);
  }
  if (SAVE_NETWORK_STRUCTURE) {
    //tw_printf(TW_LOC,"Network structure saving...\n");
    saveNeuronNetworkStructure(s);

  } else {
//#ifdef DEBUG
//    tw_printf(TW_LOC, "Not saving network structure?\n");
//#endif
  }

}

void TN_forward_event(tn_neuron_state *s, tw_bf *CV, messageData *m,
                      tw_lp *lp) {
  long start_count = lp->rng->count;
  long ld = s->myLocalID;
  long cd = s->myCoreID;
  if (VALIDATION || SAVE_MEMBRANE_POTS) {  // If we are running model validation
    // or we are saving membrane
    // potentials

    // saveNeruonState(s->myLocalID, s->myCoreID, s->membranePotential,
    // tw_now(lp));
  }
// This is the primary entry point to the neuron behavior.
// Add metrics and stats around this function.
  bool fired = TNReceiveMessage(s, m, lp, CV);
  s->SOPSCount++;
  /**@todo save message trace here: */

  CV->c0 = fired;  // save fired information for reverse computation.

  m->rndCallCount = lp->rng->count - start_count;

}

void TN_reverse_event(tn_neuron_state *s, tw_bf *CV, messageData *m,
                      tw_lp *lp) {
  if (!s->isActiveNeuron) {
    return;
  }
  long count = m->rndCallCount;
  //    tw_snapshot_restore(lp, lp->type->state_sz);

  TNReceiveReverseMessage(s, m, lp, CV);
  s->SOPSCount--;

  while (count--)
    tw_rand_reverse_unif(lp->rng);
}

/* Debug - special case - core 0 seems to be hyper-active */
#ifdef DEBUG
FILE *debug_core;
int debug_core_open = 0;
#endif

/**
 * TN_save_events - Saves an event to the output file.
 * Moved this functionality into it's own function to make testing easier.
 * Will call the neuron event save function if this neuron sent a spike AND:
 * if SAVE_SPIKE_EVTS is set,
 * or if isOutputNeuron is true AND SAVE_OUTPUT_NEURON_EVTS is set.
 * Default output core/neuron is -909 to catch possible invalid values.
 * @param s
 * @param csv
 * @param m
 * @param lp
 */
void TN_save_events(tn_neuron_state *s, tw_bf *cv, messageData *m, tw_lp *lp) {

    //if (SAVE_OUTPUT_NEURON_EVTS || SAVE_SPIKE_EVTS) { //if we are saving events, do work!
    /** @todo REMOVE THIS ONCE LIVE !!!! */

//  saveNeuronFireDebug(tw_now(lp),getCoreFromGID(lp->gid),getLocalFromGID(lp->gid),
//                      s->outputGID,getCoreFromGID(s->outputGID),getLocalFromGID(s->outputGID),s->isOutputNeuron);
        long outCore = -909;
        long outNeuron = -909;
        unsigned int fired = cv->c0 | cv->c31 | cv->c10;
        if (fired > 0) {
            outCore = s->outputCoreDest;
            outNeuron = s->outputNeuronDest;
            /** @todo: This is a debug line - this is not perm. Remove this once
             * debugging of the spike saving / data output stack is complete.
             */
            if (SAVE_SPIKE_EVTS | SAVE_NEURON_STATS | SAVE_OUTPUT_NEURON_EVTS) {
                saveNeuronFire(tw_now(lp),getCoreFromGID(lp->gid),getLocalFromGID(lp->gid),
                        s->outputGID,getCoreFromGID(s->outputGID),getLocalFromGID(s->outputGID),s->isOutputNeuron);

            }
            /////////////////////////////DEBUG END ///////////////////
            /*
            //! @todo We can use s->isOutputNeuron to check if the neuron is an "output layer" rather than relying on the bitfield
            if (SAVE_OUTPUT_NEURON_EVTS && cv->c10 &&
                !SAVE_SPIKE_EVTS) { //if save_oputput_neuron_events is active, then we save only output spikes.
                saveNeuronFire(tw_now(lp), s->myCoreID, s->myLocalID, s->outputGID, outCore, outNeuron,
                               s->isOutputNeuron);
            } else { //if SAVE_SPIKE_EVENTS is on - save all spikes. But don't call this twice (if SAVE_OUTPUT_NEURON_EVTS is on, don't write twice.
                saveNeuronFire(tw_now(lp), s->myCoreID, s->myLocalID, s->outputGID, outCore, outNeuron,
                               s->isOutputNeuron);
            }*/
        }
    //}
}

/** TN_commit is a function called on commit. This is used for management of
 * neurons! */
void TN_commit(tn_neuron_state *s, tw_bf *cv, messageData *m, tw_lp *lp) {
  // if neuron has fired and save neuron fire events is enabled, save this
  // event.
  //if (SAVE_SPIKE_EVTS && cv->c0) {
  //  saveNeuronFire(tw_now(lp), s->myCoreID, s->myLocalID, s->outputGID,getCoreFromGID(s->outputGID),getLocalFromGID(s->outputGID),0);
  //}
#ifdef DEBUG
  static int displayFlag = 0;
  if (displayFlag==0) {
    if (s->myCoreID==4041) {
      printf("------------------------------------------------------------------------------------------------\n");
      printf("Neuron 4041 commit fn. Dest neuron is %i\n", s->outputNeuronDest);
      displayFlag = 1;
    }
  }
#endif
  /// Save output neuron events
  // can save either all spike even
  TN_save_events(s,cv,m,lp);
  // save simulated dumpi trace if inter core and dumpi trace is on
  if (cv->c31 && DO_DUMPI) {
    //saveMPIMessage(s->myCoreID, getCoreFromGID(s->outputGID), tw_now(lp),

    //			   dumpi_out);
    setrnd(lp);
    saveSendMessage(s->myCoreID, getCoreFromGID(s->outputGID), tw_now(lp), 0, dumpi_out);
  }
#ifdef DEBUG
  // Debug - special log case for neuron 0
  if (!debug_core_open) {
    debug_core_open = 1;
    debug_core = fopen("debug_core_0.csv", "w");
    fprintf(debug_core, "TYPE,CORE_ID,NEURON_ID,SEND_GID,SEND_CORE,SEND_AXON,TIME,RCV_FROM_AXON\n");
  }
  if (getCoreFromGID(lp->gid)==0) { //&& cv->c0){
    // core 0 fired
    char tp;
    int recv_axon = -1;
    if (cv->c0) {
      tp = 'S';
//      fprintf(debug_core,"%c,%llu,%llu,%llu,%li,%li,%f,%i\n",tp,s->myCoreID,s->myLocalID,s->outputGID,
//              s->outputCoreDest,s->outputNeuronDest,tw_now(lp),recv_axon);
    } else {
      tp = 'R';
      recv_axon = m->localID;
    }
    fprintf(debug_core, "%c,%llu,%llu,%llu,%li,%li,%f,%i\n", tp, s->myLocalID, s->myCoreID, s->outputGID,
            s->outputCoreDest, s->outputNeuronDest, tw_now(lp), recv_axon);

  }
#endif
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
      /////////// DEBUG CODE REMOVE WHEN DONE /////////////
      debug_close_neuron_json();
    if (DO_DUMPI) {
      fclose(dumpi_out);
      fileOpen = 0;
    }
    if (SAVE_NETWORK_STRUCTURE) {
      fclose(core_connectivity_map);
    }
  }
  if (DO_DUMPI && fileOpen) {

  }
  if (g_tw_synchronization_protocol==OPTIMISTIC_DEBUG) {
    // Alpha, SOPS should be zero. HeartbeatOut should be false.
    char *em = (char *) calloc(1024, sizeof(char));
    char *hdr = "------ Neuron Optimistic Debug Check -----";
    char *alpha = "--->Membrane Potential is: ";
    char *sops = "--->SOPS is:";
    char *HB = "--->Heartbeat is:";
    bool dsp = false;
    sprintf(em, "%s\n Core: %i Local: %i \n", hdr, s->myCoreID, s->myLocalID);

    if (s->membranePotential!=0) {
      prhdr(&dsp, em);
    }
    if (s->SOPSCount!=0) {
      prhdr(&dsp, em);
      if (s->heartbeatOut!=false) {
      }
      prhdr(&dsp, em);
    }
  }
  if (SAVE_NETWORK_STRUCTURE) {
    saveIndNeuron(s);
  }
}

inline tn_neuron_state *TN_convert(void *lpstate) {
  return (tn_neuron_state *) lpstate;
}

/**@}*/
/** RIO Functions for neuron config  @{ **/
size_t tn_size(tn_neuron_state *s, tw_lp *lp) {
  size_t neuronSize = sizeof(tn_neuron_state);
  return neuronSize;
}
void tn_serialize(tn_neuron_state *s, void *buffer, tw_lp *lp) {
  memcpy(buffer, s, sizeof(tn_neuron_state));
}
void tn_deserialize(tn_neuron_state *s, void *buffer, tw_lp *lp) {
  memcpy(s, buffer, sizeof(tn_neuron_state));
}

#ifdef NET_IO_DEBUG

int tddbFileOpen = 0;



//
//void testCreateTNNeuronFromFile(tn_neuron_state *s, tw_lp *lp){
//
//	if (!tddbFileOpen){
//		sprintf(configFileName, "%s_%li.csv", BBFN, g_tw_mynode);
//		neuronConfigFile = fopen(configFileName, "w");
//		tddbFileOpen =1;
//		fprintf(neuronConfigFile, "type,isOutput,coreID,localID,"
//				"sigma0,sigma1,sigma2,sigma3,"
//				"s0,s1,s2,s3,"
//				"b0,b1,b2,b3,"
//				"sigma_lambda,"
//				"lambda,"
//				"c_lambda,"
//				"epsilon,"
//				"alpha,"
//				"beta,"
//				"TM,"
//				"gamma,"
//				"kappa,"
//				"sigma_VR,"
//				"VR,"
//				"V,"
//				"core_delay,"
//				"isSelfFiring,"
//
//				);
//		for(int i = 0; i < (NEURONS_IN_CORE * 2); i ++){
//			if(i / NEURONS_IN_CORE == 0){
//			fprintf(neuronConfigFile, "synapseConn-%i,",i%NEURONS_IN_CORE);
//			}else{
//			fprintf(neuronConfigFile, "synapse_type-%i,",i%NEURONS_IN_CORE);
//			}
//		}
//
//		UN( "isActive\n");
//	}
//	fflush(neuronConfigFile);
//	//Loop through the elements of the neuron state, saving it's configuration to the file.
//	MCRN("TN", s->isOutputNeuron,s->myCoreID,s->myLocalID);
//
//	fflush(neuronConfigFile);
//	int mode = 0;
//	int ss = 1;
//	for (int i = 0; i < (NUM_NEURON_WEIGHTS * 2); i ++){
//		if (i / NUM_NEURON_WEIGHTS == 0){
//			mode ++;
//		}
//		switch (mode) {
//			case 1:
//				//sigma0, sigma1, sigma2, sigma3 (sign of inputs from axons of type s0,s1,s2,s3
//				ss = SGN(s->synapticWeight[i % NUM_NEURON_WEIGHTS]);
//				MCRN(ss);
//				break;
//
//			case 2:
//				MCRN(s->synapticWeight[i % NUM_NEURON_WEIGHTS]);
//
//				break;
//			case 3:
//				MCRN(s->weightSelection[i % NUM_NEURON_WEIGHTS]);
//				break;
//
//		}
//	}
//
//	MCRN((int)s->sigma_l,s->lambda, s->c,s->epsilon);
//
//	MCRN(s->posThreshold, s->negThreshold, s->thresholdPRNMask, s->resetMode);
//	MCRN(s->kappa, s->sigmaVR, s->encodedResetVoltage,s->membranePotential);
//
//	MCRN((unsigned int) s->delayVal, (unsigned int)s->canGenerateSpontaniousSpikes);
//
//	fflush(neuronConfigFile);
//
//
//	for (int i = 0; i < (NEURONS_IN_CORE * 2); i ++){
//		if (i / NEURONS_IN_CORE == 0){
//			MCRN(s->synapticConnectivity[i]);
//		}else{
//			MCRN(s->axonTypes[i % NEURONS_IN_CORE]);
//		}
//	}
//
//	fflush(neuronConfigFile);
//	for(int i = 0; i < (NUM_NEURON_WEIGHTS); i++){
//		MCRN(s->weightSelection[i]);
//	}
//	fflush(neuronConfigFile);
//	UN("\n");
//	fflush(neuronConfigFile);
//
//
//
//
//}

void closeTestFile() {
  if (tddbFileOpen) {
    fclose(neuronConfigFile);

    tddbFileOpen = 0;
  }
}
#endif

