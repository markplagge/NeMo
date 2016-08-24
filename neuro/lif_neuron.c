//
// Created by Neil McGlohon on 8/9/16.
//

#include "lif_neuron.h"


void LIFFire(tn_neuron_state *st, void *l);

void LIFIntegrate(id_type snyapseID, lif_neuron_state *st, void *lp);

void LIFShouldFire(lif_neuron_state *st, tw_lp *lp);

bool LIFFireFloorCeilingReset(lif_neuron_state *st, tw_lp *lp);

void LIFNumericLeakCalc(lif_neuron_state *st, tw_stime now);

void LIFLinearLeak(lif_neuron_state *st, tw_stime now);

void LIFSendHeartbeat(lif_neuron_state *st tw_stime time, void *lp);




void LIF_init(lif_neuron_state *s, tw_lp *lp)
{
     s->neuronTypeDesc = "SIMPLE";
     if (DEBUG_MODE)
     {
          printf("Creating Neuron\n");
     }

     static int created = 0;

     bool synapticConnectivity[NEURONS_IN_CORE];
     short G_i[NEURONS_IN_CORE];
     short sigma[4]; //TODO hardcoded types of weights in neuron to 4.
     short S[4] = {[0] = 3};
     bool b[4];
     bool epsilon =0;
     bool sigma_l = 0;
     short lambda = 0;
     bool c = false;
     short TM = 0;
     short VR = 0;
     short sigmaVR = 1;
     short gamma = 0;
     bool kappa = 0;
     int signalDelay = 1;

     for(int i = 0; i < NEURONS_IN_CORE; i++)
     {
          s->axonTypes[i] = 1;
          G_i[i] = 0;
          synapticConnectivity[i] = 0;
     }
     id_type myLocalID = getNeuronLocalFromGID(lp->gid);
     synapticConnectivity[myLocalID] = 1;


     for(int i = 0; i < 4; i++) //TODO hardcoded weights in neuron to 4.
     {
          sigma[i] = 1;
          b[i] = 0;
     }

     weight_type alpha = 1;
     weight_type beta = -1;


     // *********************************** create neuron encoded rv  --- here.

     for(int i = 0; i < 4; i++)= //TODO hardcoded weights in neuron to 4.
     {
          n->snyapticWeight[i] = sigma[i] * S[i];
          n->weightSelection[i] = b[i];
     }

     for(int i = 0; i < NEURONS_IN_CORE; i++)
     {
          n->synapticConnectivity[i] = synapticConnectivity[i];
          n->axonTypes[i] = G_i[i];
     }

     //Setup other parameters
     n->myCoreID = coreID;
     n->myLocalID = nID;
     n->epsilon = epsilon;
     n->sigma_l = sigma_l;
     n->lambda = lambda;
     n->c = c;
     n->posThreshold = alpha;
     n->negThreshold = beta;
     n->sigmaVR = SGN(VR);
     n->encodedResetVoltage = VR;
     n->resetVoltage = VR; //* sigmaVR;
     n->resetMode = gamma;
     n->kappa = kappa;
     n->omega = 0;

     n->firedLast = false;
     n->heartbeatOut = false;

     //TN Set Neuron Dest
     n->delayVal = signalDelay;
     n->outputGID = gid;

     n->largestRandomValue = n->thresholdPRNMask;
     if(n->largestRandomValue > 256)
     {
          tw_error(TW_LOC, "Error - neuron (%i,%i) has a PRN Max greater than 256\n ", n->myCoreID, n->myLocalID);
     }
     //just using this rather than bit shadowing.

     n->dendriteLocal = destAxonID;
     n->outputGID = destGlobalID;

     //Check to see if we are a self-firing neuron. If so, we need to send heartbeats every big tick.
     n->isSelfFiring = false; //!@TODO: Add logic to support self-firing (spontanious) neurons

     // put the rest of create neuron encoded rv here vvv
     n->sigmaVR = sigmaVR;
     n->encodedResetVoltage = VR;
     n->resetVoltage = (n->sigmaVR * (pow(2, n->encodedResetVoltage)-1));

     if (DEBUG_MODE)
     {
          printf("Neuron type %s, num: %llu checking in with GID %llu and dest %llu\n",
           s->neuronTypeDesc, s->myLocalID, lp->gid, s->outputGID);
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


void LIF_forward_event (lif_neuron_state *s, tw_bf *CV, messageData *m, tw_lp *lp)
{
     long start_count = lp->rng->count;

     //bool fired = TNReceiveMessage()
     m->neuronVoltage = s->membranePotential;
     m->neuronLastLeakTime = s->lastLeakTime;
     m->neuronDrawnRandom = s->drawnRandomNumber;

     bool willFire = false;

     switch(m->eventType)
     {
          case SYNAPSE_OUT:
               st->drawnRandomNumber = tw_rand_integer(lp->rng, 0, st->largestRandomValue); //!<- @BUG This might be creating non-deterministic errors

               //Integrate -- AUG 23 LEFT OFF POINT
               LIFIntegrate(m->axonID,st,lp);

               if (st->heartbeatOut == false)
               {
                    tw_stime time = getNextBigTick(lp, st->myLocalID);
                    st->heartbeatOut = true;
                    bf->c13 = 1; //C13 indicates that the heartbeatout flag has been changed.
                    LIFSendHeartbeat(st, time,lp);
               }
               break;

          case NEURON_HEARTBEAT:
               st->heartbeatOut = false;
               bf->c13 = 1;

               st->drawnRandomNumber = tw_rand_integer(lp->rng,0,st->largestRandomValue);

               LIFNumericLeakCalc(st, tw_now(lp));

               willFire = LIFFireFloorCeilingReset(st, lp);
               bf->c0 = willFire;

               if(willFire)
               {
                    LIFFire(st,lp);
               }

               st->lastActiveTime = tw_now(lp);

               volt_type threshold = st->posThreshold;
               if( (st->membranePotential >= threshold) && st->heartbeatOut == false ){
                   tw_stime time = getNextBigTick(lp, st->myLocalID);
                   st->heartbeatOut = true;
                   //set message flag indicating that the heartbeat msg has been sent
                   bf->c13 = 1; //C13 indicates that the heartbeatout flag has been changed.
                   LIFSendHeartbeat(st, time, lp);
               }
               

               break;

          default:
               tw_error(TW_LOC, "Neuron (%i,%i) received invalid message type, %i \n ", st->myCoreID,st->myLocalID, m->eventType);

               break;
     }


}


void LIF_reverse_event (lif_neuron_state *s, tw_bf *CV, messageData *m, tw_lp *lp)
{

}


void LIF_commit(lif_neuron_state *s, tw_bf * cv, messageData *m, tw_lp *lp)
{

}


void LIF_final(lif_neuron_state *s, tw_lp *lp);
