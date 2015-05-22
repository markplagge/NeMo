//
// Created by plaggm on 3/18/15.
//

#include "../assist.h"
#include <stdbool.h>
#include "neuron_model.h"

void noLeak(void *neuron, tw_stime end) {
  // do nothing!!
}
void revNoLeak(void *neuron, tw_stime now) {
  // do nothing!!
}
void linearLeak(void *neuron, tw_stime end) {
  // linear leak function. The rate is determined by the leak rate in the
  // neuron.
  struct NeuronModel *s = (neuronState *)neuron;
  tw_stime delta = end - s->lastLeakTime;

  s->cVoltage -= s->leakRate * delta;
  s->lastLeakTime = end;
}

void synapticLeak(void *neuron, tw_stime end) {
  struct NeuronModel *s = (struct NeuronModel *)neuron;
  tw_stime delta = end - s->lastLeakTime;
  //	s->cVoltage +=
}
void revLinearLeak(void *neuron, tw_stime now) {
  struct NeuronModel *s = (struct NeuronModel *)neuron;
  tw_stime delta = s->lastLeakTime - now;

  s->cVoltage += s->leakRate * delta;
  s->lastLeakTime = now;
}

/**
 * Neuron Post-Fire reset functions:
 */
void resetZero(void *neuronState) {
  // State change happens here:

  struct NeuronModel *s = (struct NeuronModel *)neuronState;

  // ALL neuron functions called AFTER neuron msg rcvd and state saved.
  // s->prVoltage = s->cVoltage; // store current voltage in previous voltage
  // holder.
  s->prVoltage = s->cVoltage;
  s->cVoltage = 0; // set current voltage to 0.
}

void resetLinear(void *neuronState) {
  struct NeuronModel *s = (struct NeuronModel *)neuronState;
  // reduce the value of the neuron based on the linear reduction function
  // in the paper
  // s->cVoltage = s->cVoltage - s->resetVoltParam;
}

/**
   Neuron Post-Fire Reverse functions
 */
void reverseLinear(void *neuron) {
  struct NeuronModel *s = (struct NeuronModel *)neuron;
  // TODO - when non-zero reset functions are implemented, implement this
  // properly
  // s->cVoltage = s->cVoltage + s->resetVoltParam;
}

void reverseZero(void *neuron) {
  struct NeuronModel *s = (struct NeuronModel *)neuron;

  s->cVoltage = s->prVoltage;
  // s->cVoltage = M->prevVoltage;
}

/**
 * neuronReceiveMessage - a function that is the primary neuron message recipt
 * handler.
 * @params time The current timestamp (event timestamp)
 * @params st The state of the neuron
 * @params Msg_Data current message. Internal values will store this neuron's
 * previous state.
 * @returns bool  A bool, true if the neuron has fired.
 */
bool neuronReceiveMessage(neuronState *st, tw_stime time, Msg_Data *m,
                          tw_lp *lp) {
  bool didFire = false;
  int local = 0;

  //TODO: Use the mapping get local function isntead of re-writing code here
  local = st->neuronID + 1;

  if(local < NEURONS_IN_CORE)
    {
      tw_lpid next = globalID(st->coreID,local);
      //send a message to the next neuron - ll
      tw_stime ts = 0;
      tw_event *nextNeuron = tw_event_new(next,ts,lp);
      Msg_Data *data = (Msg_Data*) tw_event_data(nextNeuron);
      data->sender = m->sender;
      data->senderLocalID =m->senderLocalID;
      data->destCore = m->destCore;
      data->sourceCore = m->sourceCore;
      data->type = m->type;
      data->rndCallCount = lp->rng->count;
      tw_event_send(nextNeuron);
    }

  //standard integrate functions:
  st->prVoltage = st->cVoltage;
  m->prevVoltage = st->cVoltage;

  // since last communication.

  // apply weights & adjust our voltage:

  _neVoltType adjustedWeight;
  if (st->perSynapseDet[m->senderLocalID] == true) {
    adjustedWeight = st->perSynapseWeight[m->senderLocalID];
    st->cVoltage += adjustedWeight;
  } else { // stochastic mode
    _neVoltType drawnRandom = tw_rand_unif(lp->rng);
    if (drawnRandom >= st->perSynapseWeight[m->senderLocalID])
      // integrate here, stochastistyle!!
      st->cVoltage += st->perSynapseWeight[m->senderLocalID];
  }
  st->leak(st, time); // leak function call - do leak based on time passed
  st->integrationCount++;
  // next check for fire operations:
  switch (st->fireMode) {
    case NFM:
    default:
      if (st->cVoltage >= st->threshold) {
        didFire = true;
        neuronFire(st, time, m);
      }
  }
  return didFire;


}

bool neuronReceiveMessageOld(neuronState *st, tw_stime time, Msg_Data *m,
                          tw_lp *lp) {

  bool didFire = false;
  bool isTimeToFire = false;
  tw_stime chgVal =
      tw_now(lp) * ((float)st->burstRate / (float)st->neuronsInCore);
  tw_stime nowtest = tw_now(lp);

  if ((fabs(roundf(chgVal) - chgVal) <= 0.01f) && tw_now(lp) > 1)
    isTimeToFire = true;

  if (m->type != NEU_WAIT) {
    // prep the rotors ( reverse functions )
    st->prVoltage = st->cVoltage;
    m->prevVoltage = st->cVoltage;

    // since last communication.

    // apply weights & adjust our voltage:

    _neVoltType adjustedWeight;
    if (st->perSynapseDet[m->senderLocalID] == true) {
      adjustedWeight = st->perSynapseWeight[m->senderLocalID];
      st->cVoltage += adjustedWeight;
    } else { // stochastic mode
      _neVoltType drawnRandom = tw_rand_unif(lp->rng);
      if (drawnRandom >= st->perSynapseWeight[m->senderLocalID])
        // integrate here, stochastistyle!!
        st->cVoltage += st->perSynapseWeight[m->senderLocalID];
    }
  }

  // Check for valid time (based on burst rate) for integration count and fire
  // operations:
  if (isTimeToFire == true) {
    // prep complete. Now leaking based on time lapse:
    st->leak(st, time); // leak function call - do leak based on time passed
    st->integrationCount++;
    // next check for fire operations:
    switch (st->fireMode) {
      case NFM:
      default:
        if (st->cVoltage >= st->threshold) {
          didFire = true;
          neuronFire(st, time, m);
        }
    }
  } else {
    generateWaitEvent(st, lp);
  }
  if (didFire)
    neuronPostFire(st, time, m);
  return didFire;
}

/** neuronPostFire - Function that cleans up the neuron state after firing. */

void neuronPostFire(neuronState *st, tw_stime time, Msg_Data *m) {
  st->lastActiveTime = time;

  st->doReset(st); // that may be a little fugly, but it does allow swapping of
                   // behaviors at runtime.
}

/**
 * @brief neuronFire Function called after firing status is determined to be
 * true.
 * Actual message is managed through model_main.c. This function adjusts
 * parameters for tracking neuron behaviors.
 * @param st
 * @param time
 * @param m
 */
void neuronFire(neuronState *st, tw_stime time, Msg_Data *m) {
  st->lastActiveTime = time;
  st->fireCount++;
}

void neuronReverseFinal(neuronState *s, tw_bf *CV, Msg_Data *m, tw_lp *lp) {
  // state was recovered by the reverse fire and reverse reset functions.
  // here, we have to roll back the state to ensure that the stochastic mode
  // neuron
  // rolls back the random function properly.

  // reverse neuron state function
  // do functions in reverse order:
  // 1. Reset State reverse
  s->reverseReset(s);
  // 2. Leak reverse
  s->reverseLeak(s, tw_now(lp));
  // 3. Fire/Random reverse Reverse:
  long count = m->rndCallCount;
  while (count--)
    tw_rand_reverse_unif(lp->rng);
}
void generateWaitEvent(neuronState *st, tw_lp *lp) {
  // If we can't integrate yet, then we need to send a message to integrate when
  // it is time.

  tw_stime ts = getNextEventTime(lp);
  // make this more eff.
  int now = tw_now(lp);
  if (now == 0) {
    ts += st->neuronsInCore / st->burstRate;
  } else
    do {
      ts += st->neuronsInCore / st->burstRate;
      now += st->neuronsInCore / st->burstRate;
    } while (now % (st->neuronsInCore / st->burstRate));
  tw_event *evt = tw_event_new(lp->gid, ts, lp);
  Msg_Data *data = (Msg_Data *)tw_event_data(evt);
  data->senderLocalID = st->neuronID;
  data->type = NEU_WAIT;
  data->destCore = st->coreID;
  data->rndCallCount = lp->rng->count;
  tw_event_send(evt);
}
