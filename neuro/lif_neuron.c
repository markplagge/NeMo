//
// Created by Neil McGlohon on 8/9/16.
//

#include "lif_neuron.h"


void LIFFire(tn_neuron_state *s, tw_lp *lp));

void LIFIntegrate(id_type snyapseID, lif_neuron_state *st, tw_lp *lp));

void LIFShouldFire(lif_neuron_state *s, tw_lp *lp);

void LIFLeakProcess(lif_neuron_state *s, tw_stime now);

void LIFSendHeartbeat(lif_neuron_state *s tw_stime time, tw_lp *lp));

void LIF_init(lif_neuron_state *s, tw_lp *lp)
{

     //residual TN LIF model stuff / NeMo stuff
     if (DEBUG_MODE)
     {
          printf("Creating Neuron\n");
     }

     static int created = 0;
     short sigma[4]; //TODO hardcoded types of weights in neuron to 4.
     short S[4] = {[0] = 3};
     bool b[4];
     int signalDelay = 1;
     bool heartbeatOut = false;

     //my model stuff
     s->R_mem = 10;
     s->C_mem = 25;
     s->Tau = R_mem * C_mem;
     s->V_thresh = 1.6;
     s->V_in = 0.0;
     s->V_spike = .25;
     s->V_mem = 0;
     s->V_last = 0;
     s->firing_count = 0;

     s->R_mem = 10;



     for(int i = 0; i < NEURONS_IN_CORE; i++)
     {
          s->axonTypes[i] = 0;
          s->synapticConnectivity[i] = 0;
     }
     id_type myLocalID = getNeuronLocalFromGID(lp->gid);
     s->synapticConnectivity[myLocalID] = 1;



     for(int i = 0; i < 4; i++) //TODO hardcoded weights in neuron to 4.
     {
          sigma[i] = 1;
          b[i] = 0;
     }

     for(int i = 0; i < 4; i++)= //TODO hardcoded weights in neuron to 4.
     {
          s->snyapticWeight[i] = sigma[i] * S[i];
          s->weightSelection[i] = b[i];
     }

     //Setup other parameters
     s->myCoreID = coreID;
     s->myLocalID = nID;

     s->firedLast = firedLast;
     s->heartbeatOut = heartbeatOut;

     //TN Set Neuron Dest
     s->delayVal = signalDelay;
     s->outputGID = gid;

     s->dendriteLocal = destAxonID;
     s->outputGID = destGlobalID;


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
          s->V_in += weight/s->Tau;
     }
}

void LIFLeakProcess(lif_neuron_state *s, tw_stime now)
{
     uint_fast32_t numBigTicksMissed = getCurrentBigTick(now) - getCurrentBigTick(s->lastLeakTime); //minimum is 1
     volt_type newVoltage = s->V_mem;
     for(int i = 0; i < numBigTicksMissed; i++)
     {
          newVoltage = newVoltage - newVoltage/s->Tau;
     }

     s->V_mem = newVoltage
     s->lastLeakTime = now;


}

void LIF_forward_event (lif_neuron_state *s, tw_bf *CV, messageData *m, tw_lp *lp)
{
     long start_count = lp->rng->count;

     m->neuronVoltage = s->V_mem;
     m->neuronLastLeakTime = s->lastLeakTime;

     bool willFire = false;

     switch(m->eventType)
     {
          case SYNAPSE_OUT:
               LIFIntegrate(m->axonID,st,lp);

               if (st->heartbeatOut == false)
               {
                    tw_stime time = getNextBigTick(lp, s->myLocalID);
                    s->heartbeatOut = true;
                    bf->c13 = 1; //C13 indicates that the heartbeatout flag has been changed.
                    LIFSendHeartbeat(st, time,lp);
               }
               break;

          case NEURON_HEARTBEAT:
               s->heartbeatOut = false;
               bf->c13 = 1;

               LIFLeakProcess(s, tw_now(lp));

               willFire = LIFShouldFire(s, lp);
               CV->c0 = willFire;
               if(willFire)
               {
                    LIFFire(s,lp);
               }

               s->lastActiveTime = tw_now(lp);


               s->V_in = 0; //Reset the input voltage from the integration steps
               break;

          default:
               tw_error(TW_LOC, "Neuron (%i,%i) received invalid message type, %i \n ", st->myCoreID,st->myLocalID, m->eventType);
               break;
     }

     s->SOPSCount++;
     m->rndCallCount = lp->rng->count - start_count;
}


void LIF_reverse_event (lif_neuron_state *s, tw_bf *CV, messageData *m, tw_lp *lp)
{
     long count = m->rndCallCount;

     if (M->eventType == NEURON_HEARTBEAT) {
       // reverse heartbeat message
       // st->SOPSCount--;
     }
     if (bf->c0) {  // c0 flags firing state
                    // reverse computation of fire and reset functions here.
       /**@todo implement neuron fire/reset reverse computation functions */
     }
     if (bf->c13) {
       s->heartbeatOut = s->heartbeatOut;
     }
     /**@todo remove this once neuron reverse computation functions are built. */
     s->V_mem = m->neuronVoltage;
     s->lastLeakTime = m->neuronLastLeakTime;
     s->lastActiveTime = m->neuronLastActiveTime;

     s->SOPSCount--;

     while (count--) tw_rand_reverse_unif(lp->rng);
}


void LIF_commit(lif_neuron_state *s, tw_bf * cv, messageData *m, tw_lp *lp)
{
     if (SAVE_SPIKE_EVTS && cv->c0) {
       saveNeuronFire(tw_now(lp), s->myCoreID, s->myLocalID, s->outputGID);
     }
}


void LIF_final(lif_neuron_state *s, tw_lp *lp)
{
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
