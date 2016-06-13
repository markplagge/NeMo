#include "synapse.h"


void synapse_init(synapseState *s, tw_lp *lp){
	s->msgSent = 0;
	s->lastBigTick = 0;
	s->myCore = getCoreFromGID(lp->gid);
}

void synapse_event(synapseState *s, tw_bf *, messageData *M, tw_lp *lp){
	long rc = lp->rng->count;
	
	messageData outMessage;
	tw_event *synEvt;
	for(int i = 0; i < NEURONS_IN_CORE; i ++){
		//add the check for connected neurons here
		
		synEvt = tw_event_new(getNeuronGlobal(s->myCore, i), getNextEventTime(lp), lp);
		outMessage = (messageData *) tw_event_data(synEvt)
		outMessage->eventType = SYNAPSE_OUT;
		outMessage->axonID = M->axonID;

		
		M->rndCallCount = lp->rng->count - rc;
	
		s->msgSent ++;
		tw_event_send(synEvt);
	}
	
	
}
void synapse_reverse(synapseState *, tw_bf *, messageData *M, tw_lp *lp){
	s->msgSent --;

	unsigned long count = m->rndCallCount;
	while (count --){
		tw_rand_reverse_unif(lp->rng);
	}
}

void synapse_final(synapseState *s, tw_lp *lp){
	//do some stats here if needed.
}

/** New Super Synapse with fanout code
void super_synpase_event(synapseState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp){
    //Set up random counter monitor.
    long rc = lp->rng->count;

    unsigned long neuronCount ;

    //store neuron dest count inside the message, under axonID or something so rewrites are not as extensive.
    if (M->eventType == AXON_OUT){
        neuronCount = NEURONS_IN_CORE;
    }
    else{
        neuronCount = M->synapseHeartbeatCounter;
    }
    tw_event *av;
    MsgData *data;
    if (neuronCount > 0){


    //Create a heartbeat Synapse message
        //set up the synapse heartbeat
        *av = tw_event_new(lp->gid, getNextEventTime(lp), lp);
        *data = (Msg_Data *) tw_event_data(av);
        data->eventType = SYNAPSE_OUT;
        data->localID = lp->gid;
        data->axonID = M->axonID;
        neuronCount --;
        data->synapseHeartbeatCounter = neuronCount;
        tw_event_send(av);
    }

    // right now - I'm going to send messages from N to 0
    tw_lpid currentNeuronDest = neuronCount; // NeuronCount + GID of some sort.
    *av = tw_event_new(currentNeuronDest, getNextEventTime(lp),lp);
    *data = (Msg_Data *) tw_event_data(av);
    data->event = SYNAPSE_OUT;
    data->localID = s->mySynapseNum;
    data->axonID = M->axonID;
    tw_event_send(axe);
    M->rndCallCount = lp->rng->count - rc;

 } */