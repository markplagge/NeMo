//
// Created by Neil McGlohon on 8/9/16.
//

#include "lif_neuron.h"


void LIFFire(lif_neuron_state *s, tw_lp *lp);

void LIFIntegrate(id_type snyapseID, lif_neuron_state *s, tw_lp *lp);

bool LIFShouldFire(lif_neuron_state *s, tw_lp *lp);

void LIFLeakProcess(lif_neuron_state *s, tw_stime now);

void LIFSendHeartbeat(lif_neuron_state *s, tw_stime time, tw_lp *lp);

void LIF_init(lif_neuron_state *s, tw_lp *lp)
{

     //residual TN LIF model stuff / NeMo stuff
     if (DEBUG_MODE)
     {
          printf("Creating Neuron\n");
     }

     static int created = 0;
     int signalDelay = 1;

     //my model stuff
     s->R_mem = 10;
     s->C_mem = 25;
     s->Tau = s->R_mem * s->C_mem;
     s->V_thresh = .3;
     s->V_in = 0.0;
     s->V_spike = .25;
     s->V_mem = 0;
     s->V_history = calloc((int)g_tw_ts_end, sizeof(volt_type));
     s->firing_count = 0;
     s->outputGID = 0;
     s->delayVal = signalDelay;



     for(int i = 0; i < NEURONS_IN_CORE; i++)
     {
          s->axonTypes[i] = tw_rand_integer(lp->rng, 0, NUM_NEURON_WEIGHTS - 1);
          s->synapticConnectivity[i] = 0;
     }
     id_type myLocalID = getNeuronLocalFromGID(lp->gid);
     s->synapticConnectivity[myLocalID] = 1;


     for(int i = 0; i < NUM_NEURON_WEIGHTS; i++) //TODO hardcoded weights in neuron to 4.
     {
          s->synapticWeight[i] = tw_rand_unif(lp->rng) * 10;
     }

     //Setup other parameters
     s->myCoreID = getCoreFromGID(lp->gid);
     s->myLocalID = getNeuronLocalFromGID(lp->gid);

     s->heartbeatOut = false;

     if(s->myLocalID == 0)
     {
          printf("weights for gid 0 are:\n");
          for(int i = 0; i< NUM_NEURON_WEIGHTS; i++)
          {
               printf("%f\n",s->synapticWeight[i]);
          }
     }


     if (DEBUG_MODE)
     {
          printf("Neuron type SIMPLE, num: %llu checking in with GID %llu and dest %llu\n",
          s->myLocalID, lp->gid, s->outputGID);
     }

     float remoteCoreProbability = .905;
     long int dendriteCore = s->myCoreID;
     //This neuron's core is X. There is a 90.5% chance that my destination will be X - and a 10% chance it will be a different core.
     if(tw_rand_unif(lp->rng) < remoteCoreProbability)
     {
          dendriteCore =  tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);
     }

     /**@note This random setup will create neurons that have an even chance of getting an axon inside thier own core
     * vs an external core. The paper actually capped this value at something like 20%. @todo - make this match the
     * paper if performance is slow. * */
     //s->dendriteCore = tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);
     s->dendriteLocal = s->myLocalID;// tw_rand_integer(lp->rng, 0, AXONS_IN_CORE - 1);
     //     if (tnMapping == LLINEAR) {
     s->outputGID = getAxonGlobal(dendriteCore, s->dendriteLocal);
     created++;
}

void LIFIntegrate(id_type synapseID, lif_neuron_state* s, tw_lp *lp)
{
     bool con = s->synapticConnectivity[synapseID];

     if(con) //if there's a connection
     {
          weight_type weight = s->synapticWeight[s->axonTypes[synapseID]];
          s->V_in += weight * s->R_mem;
     }
}

void LIFLeakProcess(lif_neuron_state *s, tw_stime now)
{
     uint_fast32_t numBigTicksMissed = getCurrentBigTick(now) - getCurrentBigTick(s->lastLeakTime); //minimum is 1
     volt_type newVoltage = s->V_mem + s->V_in/s->Tau;
     for(int i = 0; i < numBigTicksMissed; i++)
     {
          newVoltage = newVoltage - newVoltage/s->Tau;
     }

     s->V_mem = newVoltage;
     s->lastLeakTime = now;


}

bool LIFShouldFire(lif_neuron_state* s, tw_lp* lp) {
  // check negative threshold values:
  volt_type threshold = s->V_thresh;
  return (s->V_mem >= threshold);  // + (s->drawnRandomNumber));
}

void LIFFire(lif_neuron_state *s, tw_lp *lp) {

  // printf("%i: I am firing!\n",s->myLocalID);

  //Send fire message to the output LP for this neuron
  tw_stime nextHeartbeat = getNextBigTick(lp, s->myLocalID);
  tw_event* newEvent = tw_event_new(s->outputGID, nextHeartbeat, lp);
  messageData* data = (messageData*)tw_event_data(newEvent);
  data->eventType = NEURON_OUT;
  data->localID = s->myLocalID;
  tw_event_send(newEvent);
}

void LIFSendHeartbeat(lif_neuron_state* s, tw_stime time, tw_lp* lp) {
  tw_event* newEvent =
      tw_event_new(lp->gid, getNextBigTick(lp, s->myLocalID), lp);
  // tw_event *newEvent = tw_event_new(lp->gid, (0.1 + (tw_rand_unif(lp->rng) /
  // 1000)),lp);
  messageData* data = (messageData*)tw_event_data(newEvent);
  data->localID = s->myLocalID;
  data->eventType = NEURON_HEARTBEAT;
  tw_event_send(newEvent);
  if (s->heartbeatOut == false) {
    tw_error(TW_LOC,
             "Error - neuron sent heartbeat without setting HB to true\n");
  }
}

void LIF_forward_event (lif_neuron_state *s, tw_bf *bf, messageData *m, tw_lp *lp)
{
     long start_count = lp->rng->count;

     m->neuronVoltageIn = s->V_in;
     m->neuronVoltage = s->V_mem;
     m->neuronLastLeakTime = s->lastLeakTime;
     m->neuronLastActiveTime = s->lastActiveTime;

     bool willFire = false;

     switch(m->eventType)
     {
          case SYNAPSE_OUT:
               // if(s->myLocalID == 510)
               // {
               //      // printf("%i: I received a firing from %i!\n",s->myLocalID,m->localID);
               // }


               LIFIntegrate(m->axonID,s,lp);

               if (s->heartbeatOut == false)
               {
                    tw_stime time = getNextBigTick(lp, s->myLocalID);
                    s->heartbeatOut = true;
                    bf->c13 = 1; //C13 indicates that the heartbeatout flag has been changed.
                    LIFSendHeartbeat(s, time,lp);
               }
               break;

          case NEURON_HEARTBEAT:
               // if(s->myLocalID == 510)
               // {
               //      printf("%i: I received a heartbeat from %i!\n",s->myLocalID,m->localID);
               // }
               s->heartbeatOut = false;
               bf->c13 = 1;

               LIFLeakProcess(s, tw_now(lp));

               willFire = LIFShouldFire(s, lp);
               bf->c0 = willFire;
               if(willFire)
               {
                    LIFFire(s,lp);
               }

               s->lastActiveTime = tw_now(lp);


               s->V_in = 0; //Reset the input voltage from the integration steps
               s->V_history[(int)tw_now(lp)] = s->V_mem;
               if(willFire)
               {
                    // s->V_mem = 0;
               }
               break;

          default:
               tw_error(TW_LOC, "Neuron (%i,%i) received invalid message type, %i \n ", s->myCoreID,s->myLocalID, m->eventType);
               break;
     }

     s->SOPSCount++;
     m->rndCallCount = lp->rng->count - start_count;
}


void LIF_reverse_event (lif_neuron_state *s, tw_bf *bf, messageData *m, tw_lp *lp)
{
     long count = m->rndCallCount;

     if (m->eventType == NEURON_HEARTBEAT) {
       // reverse heartbeat message
       // s->SOPSCount--;
     }
     if (bf->c0) {  // c0 flags firing state
                    // reverse computation of fire and reset functions here.
       /**@todo implement neuron fire/reset reverse computation functions */
     }
     if (bf->c13) {
       s->heartbeatOut = !s->heartbeatOut;
     }
     /**@todo remove this once neuron reverse computation functions are built. */
     s->V_mem = m->neuronVoltage;
     s->V_in = m->neuronVoltageIn;
     s->lastLeakTime = m->neuronLastLeakTime;
     s->lastActiveTime = m->neuronLastActiveTime;

     s->SOPSCount--;

     while (count--) tw_rand_reverse_unif(lp->rng);
}


void LIF_commit(lif_neuron_state *s, tw_bf * bf, messageData *m, tw_lp *lp)
{
     if (SAVE_SPIKE_EVTS && bf->c0) {
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

void LIF_final(lif_neuron_state *s, tw_lp *lp)
{
     // if(s->myLocalID == 0)
     // {
     //      for(int i = 0; i < g_tw_ts_end; i++)
     //      {
     //           printf("%i: %f\n",i,s->V_history[i]);
     //      }
     // }

     if (g_tw_synchronization_protocol == OPTIMISTIC_DEBUG) {
       // Alpha, SOPS should be zero. HeartbeatOut should be false.
       char* em = (char*)calloc(1024, sizeof(char));
       char* hdr = "------ Neuron Optimistic Debug Check -----";
       char* alpha = "--->Membrane Potential is: ";
       char* sops = "--->SOPS is:";
       char* HB = "--->Heartbeat is:";
       bool dsp = false;

       sprintf(em, "%s\n Core: %i Local: %i \n", hdr, s->myCoreID, s->myLocalID);
       if (s->V_mem != 0) {
         prhdr(&dsp, em);
         debugMsg(alpha, s->V_mem);
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
