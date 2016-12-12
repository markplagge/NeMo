//
// Created by Mark Plagge on 5/25/16.
//

#include "liftn_neuron.h"

/** \defgroup TN_Function_hdrs True North Function headers
 * TrueNorth Neuron leak, integrate, and fire function forward decs.
 * @{ */

void LIFFire(lif_neuron_state* st, void* l);
/**
 *  @brief  function that adds a synapse's value to the current neuron's
 * membrane potential.
 *
 *  @param synapseID localID of the synapse sending the message.
 */
void LIFIntegrate(id_type synapseID, lif_neuron_state* st, void* lp);
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

bool LIFReceiveMessage(lif_neuron_state* st, messageData* M, tw_lp* lp,
                      tw_bf* bf);

/**
 * @brief handels reverse computation and state messages.
 * @param st current neuron state
 * @param M reverse message
 * @param lp the lp
 * @param bf the reverse computation bitfield.
 */

void LIFReceiveReverseMessage(lif_neuron_state* st, messageData* M, tw_lp* lp,
                             tw_bf* bf);

/**
 *  @brief  Checks to see if a neuron should fire.
 *  @todo check to see if this is needed, since it looks like just a simple if
 * statement is in order.
 *
 *  @param st neuron state
 *
 *  @return true if the neuron is ready to fire.
 */
bool LIFShouldFire(lif_neuron_state* st, tw_lp* lp);

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
// void LIFNumericLeakCalc(lif_neuron_state* st, tw_stime now);

void LIFConstantLeak(lif_neuron_state* st, tw_stime now);



void LIFSendHeartbeat(lif_neuron_state* st, tw_stime time, void* lp);


void LIF_reset_membrane(lif_neuron_state* st, tw_lp* lp)
{
     st-> membranePotential = 0;
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
void LIFFire(lif_neuron_state* st, void* l) {
  tw_lp* lp = (tw_lp*)l;
  // DEBUG
  //	tw_lpid outid = st->dendriteGlobalDest;
  //	tw_lp *destLP = tw_getlp(outid);
  //	printf("Sending message to %llu\n", destLP->gid);

  // DEBUG

  //Send fire message to the output LP for this neuron
  tw_stime nextHeartbeat = getNextBigTick(lp, st->myLocalID);
  tw_event* newEvent = tw_event_new(st->outputGID, nextHeartbeat, lp);
  messageData* data = (messageData*)tw_event_data(newEvent);
  data->eventType = NEURON_OUT;
  data->localID = st->myLocalID;
  tw_event_send(newEvent);
  st->firedLast = true;
}
bool LIFReceiveMessage(lif_neuron_state* st, messageData* m, tw_lp* lp,
                      tw_bf* bf) {
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

    case SYNAPSE_OUT: //You received a firing
      LIFIntegrate(m->axonID, st, lp);
      // next, we will check if a heartbeat message should be sent
      if (st->heartbeatOut == false) {
        tw_stime time = getNextBigTick(lp, st->myLocalID);
        st->heartbeatOut = true;
        bf->c13 =
            1;  // C13 indicates that the heartbeatout flag has been changed.
        LIFSendHeartbeat(st, time, lp);

        // set message flag indicating that the heartbeat msg has been sent
      }
      break;

    case NEURON_HEARTBEAT:
      st->heartbeatOut = false;
      // set message flag indicating that the heartbeat msg has been sent
      bf->c13 =
          1;  // C13 indicates that the heartbeatout flag has been changed.


      LIFConstantLeak(st, tw_now(lp));
      // linearLeak( st, tw_now(lp));


      // willFire = neuronShouldFire(st, lp); //removed and replaced with
      // fireFloorCelingReset
      willFire = LIFShouldFire(st,lp);
      bf->c0 = willFire;

      if (willFire) {
        LIFFire(st, lp);
        // st->fireCount++;
      }

      // neuronPostIntegrate(st, tw_now(lp), lp, willFire); //removed and
      // replaced with fireFloorCelingReset
      // stats collection
      // st->SOPSCount++;
      st->lastActiveTime = tw_now(lp);


      // do we still have more than the threshold volts left? if so,
      // send a heartbeat out that will ensure the neuron fires again.
      // Or if we are as self-firing neuron.
      ///@TODO: Add detection of self-firing neuron state.
      ///@TODO: Ensure bf-c13 state validity here for reverse computations
      if (LIFShouldFire(st, lp) && st->heartbeatOut == false) {
        tw_stime time = getNextBigTick(lp, st->myLocalID);
        st->heartbeatOut = true;
        // set message flag indicating that the heartbeat msg has been sent
        bf->c13 =
            1;  // C13 indicates that the heartbeatout flag has been changed.
        LIFSendHeartbeat(st, time, lp);
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
    LIFSendHeartbeat(st, time, lp);
  }
  return willFire;
}
void LIFReceiveReverseMessage(lif_neuron_state* st, messageData* M, tw_lp* lp,
                             tw_bf* bf) {
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
}


void LIFIntegrate(id_type synapseID, lif_neuron_state* st, void* lp) {
  bool con = st->synapticConnectivity[synapseID];
  if (con == 0) return;
  if(st->synapticConnectivity[synapseID]){
       weight_type weight = st->synapticWeight[st->axonTypes[synapseID]];
       st->membranePotential += weight;
  }
}


void LIFSendHeartbeat(lif_neuron_state* st, tw_stime time, void* lp) {
  tw_lp* l = (tw_lp*)lp;
  tw_event* newEvent =
      tw_event_new(l->gid, getNextBigTick(l, st->myLocalID), l);
  // tw_event *newEvent = tw_event_new(l->gid, (0.1 + (tw_rand_unif(l->rng) /
  // 1000)),l);
  messageData* data = (messageData*)tw_event_data(newEvent);
  data->localID = st->myLocalID;
  data->eventType = NEURON_HEARTBEAT;
  tw_event_send(newEvent);
  if (st->heartbeatOut == false) {
    tw_error(TW_LOC,
             "Error - neuron sent heartbeat without setting HB to true\n");
  }
}

bool LIFShouldFire(lif_neuron_state* st, tw_lp* lp) {
  // check negative threshold values:
  volt_type threshold = st->posThreshold;
  return (st->membranePotential >= threshold);  // + (st->drawnRandomNumber));
}




void LIFConstantLeak(lif_neuron_state* st, tw_stime now)
{
     // calculate current time since last leak --- LEAK IS TERRIBLE FOR THIS:
     uint_fast32_t numberOfBigTicksSinceLastLeak =
         getCurrentBigTick(now) - getCurrentBigTick(st->lastLeakTime);
     // then run the leak function until we've caught up:
     for(; numberOfBigTicksSinceLastLeak > 0; numberOfBigTicksSinceLastLeak--)
     {
          st->membranePotential = st->membranePotential - st->lambda_j;
     }
     st->lastLeakTime = now;
}


void LIF_set_neuron_dest(int signalDelay, uint64_t gid, lif_neuron_state* n) {
  n->delayVal = signalDelay;
  n->outputGID = gid;
}


/** Constructor / Init a new neuron. assumes that the reset voltage is NOT
 * encoded (i.e.,
  * a reset value of -5 is allowed. Sets reset voltage sign from input reset
 * voltage).*/
void LIF_create_neuron(id_type coreID, id_type nID,
                      bool synapticConnectivity[NEURONS_IN_CORE],
                      short G_i[NEURONS_IN_CORE], short sigma[4], short S[4],
                      bool b[4], short sigma_l, short lambda_j, uint32_t alpha,
                      lif_neuron_state* n, int signalDelay,
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
  n->sigma_l = sigma_l;
  n->lambda_j = lambda_j;
  n->posThreshold = alpha;

  n->firedLast = false;
  n->heartbeatOut = false;

  LIF_set_neuron_dest(signalDelay, destGlobalID, n);

  // synaptic neuron setup:
  n->dendriteLocal = destAxonID;
  n->outputGID = destGlobalID;

  // Check to see if we are a self-firing neuron. If so, we need to send
  // heartbeats every big tick.
  n->isSelfFiring =
      false;  //!@TODO: Add logic to support self-firing (spontanious) neurons
}
// void LIF_create_neuron_encoded_rv(
//     id_type coreID, id_type nID, bool synapticConnectivity[NEURONS_IN_CORE],
//     short G_i[NEURONS_IN_CORE], short sigma[4], short S[4], bool b[4],
//     bool epsilon, short sigma_l, short lambda, bool c, uint32_t alpha,
//     uint32_t beta, short TM, short VR, short sigmaVR, short gamma, bool kappa,
//     tn_neuron_state* n, int signalDelay, uint64_t destGlobalID,
//     int destAxonID) {
//  LIF_create_neuron(coreID, nID, synapticConnectivity, G_i, sigma, S, b, epsilon,
//                    sigma_l, lambda, c, alpha, beta, TM, VR, sigmaVR, gamma,
//                    kappa, n, signalDelay, destGlobalID, destAxonID);
//   n->sigmaVR = sigmaVR;
//   n->encodedResetVoltage = VR;
//   n->resetVoltage = (n->sigmaVR * (pow(2, n->encodedResetVoltage) - 1));
// }
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
void LIF_create_simple_neuron(lif_neuron_state* s, tw_lp* lp) {
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
  bool sigma_l = 0;
  short lambda_j = .1;
  int signalDelay = 1;  // tw_rand_integer(lp->rng, 0,5);
  weight_type alpha = 1;


  for (int i = 0; i < NEURONS_IN_CORE; i++) {
    s->axonTypes[i] = 1;
    G_i[i] = 0;
    synapticConnectivity[i] = 0;
  }

  id_type myLocalID = getNeuronLocalFromGID(lp->gid);

  synapticConnectivity[myLocalID] = 1;

  //(creates an "ident. matrix" of neurons.
  for (int i = 0; i < 4; i++) {
    sigma[i] = 1;
    b[i] = 0;
  }

  LIF_create_neuron(
      getCoreFromGID(lp->gid), getNeuronLocalFromGID(lp->gid),
      synapticConnectivity, G_i, sigma, S, b, sigma_l, lambda_j, alpha,
      s, signalDelay, 0, 0);
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


void LIF_init(lif_neuron_state* s, tw_lp* lp) {
  static int pairedNeurons = 0;
  static bool announced = false;
  if (DEBUG_MODE && !announced) {
    printf("Creating neurons\n");
    announced = true;
  }
  // ADD FILE INPUT NEURON CREATION HERE

  LIF_create_simple_neuron(s, lp);

  if (DEBUG_MODE) {
    printf(
        "Neuron type SIMPLE, num: %hu checking in with GID %llu and dest %llu \n", s->myLocalID, lp->gid, s->outputGID);
  }
}


void LIF_forward_event(lif_neuron_state* s, tw_bf* CV, messageData* m,
                      tw_lp* lp) {
  long start_count = lp->rng->count;

  bool fired = LIFReceiveMessage(s, m, lp, CV);
  s->SOPSCount++;
  /**@todo save message trace here: */

  CV->c0 = fired;  // save fired information for reverse computation.

  m->rndCallCount = lp->rng->count - start_count;
}

void LIF_reverse_event(lif_neuron_state* s, tw_bf* CV, messageData* m,
                      tw_lp* lp) {
  long count = m->rndCallCount;

  LIFReceiveReverseMessage(s, m, lp, CV);
  s->SOPSCount--;

  while (count--) tw_rand_reverse_unif(lp->rng);
}

/** TN_commit is a function called on commit. This is used for management of
 * neurons! */
void LIF_commit(lif_neuron_state* s, tw_bf* cv, messageData* m, tw_lp* lp) {
  // if neuron has fired and save neuron fire events is enabled, save this
  // event.
  if (SAVE_SPIKE_EVTS && cv->c0) {
    saveNeuronFire(tw_now(lp), s->myCoreID, s->myLocalID, s->outputGID);
  }
}
/** @todo: fix this remote value */
void prhdr(bool* display, char* hdr) {
  if (&display) {
    print(hdr);
    *display = true;
  }
}
void LIF_final(lif_neuron_state* s, tw_lp* lp) {
  if (g_tw_synchronization_protocol == OPTIMISTIC_DEBUG) {
    // Alpha, SOPS should be zero. HeartbeatOut should be false.
    char* em = (char*)calloc(1024, sizeof(char));
    char* hdr = "------ Neuron Optimistic Debug Check -----";
    char* alpha = "--->Membrane Potential is: ";
    char* sops = "--->SOPS is:";
    char* HB = "--->Heartbeat is:";
    bool dsp = false;

    sprintf(em, "%s\n Core: %i Local: %i \n", hdr, s->myCoreID, s->myLocalID);
    if (s->membranePotential != 0) {
      prhdr(&dsp, em);
      debugMsg(alpha, s->membranePotential);
    }
    if (s->SOPSCount != 0) {
      prhdr(&dsp, em);
      debugMsg(sops, s->SOPSCount);
    }
    if (s->heartbeatOut != false) {
      prhdr(&dsp, em);
      debugMsg(HB, (int)s->heartbeatOut);
    }
  }
}

inline lif_neuron_state* LIF_convert(void* lpstate) {
  return (lif_neuron_state*)lpstate;
}
