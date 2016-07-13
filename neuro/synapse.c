#include "synapse.h"


void synapse_init(synapseState *s, tw_lp *lp){
	s->msgSent = 0;
	s->lastBigTick = 0;
	s->myCore = getCoreFromGID(lp->gid);
	// The random benchmark network uses an "identity matrix" of axon->neuron connectivity for now.
	// So if we are using this type of network, set the diagonal of this synapse's connectivity grid to 1.
	if (!FILE_IN){
		for(int i = 0; i< AXONS_IN_CORE; i ++ ){
			for(int j = 0; j < NEURONS_IN_CORE; j++) {
				if (i == j){
					s->connectionGrid[i][j] = 1;
				}
				else
					s->connectionGrid[i][j] = 0;
			}
		}
	}
	
	
	if(DEBUG_MODE){
		printf("Super Synapse Created - GID is %llu\n", lp->gid);
	}
}
#ifdef  SAVE_MSGS
void saveSynapseMessage(messageData *M, tw_lp *lp){
	
}

#endif
void synapse_event(synapseState *s, tw_bf *bf, messageData *M, tw_lp *lp){
	long rc = lp->rng->count;
/** TODO: This is probably not going to work. I think the synapse will need a larger state. */


		messageData *outMessage;
		tw_event *synEvt;

	if (M->eventType == AXON_OUT:){
		synEvt = tw_event_new(lp->gid, getNextEventTime(lp),lp );
		outMessage = (messageData *) tw_event_data(synEvt);
		outMessage->eventType = SYNAPSE_HEARTBEAT;
		outMessage->synapseCount = 0;
		outMessage->axonID = M->axonID;
#ifdef SAVE_MSGS
		saveSynapseMessage(M, lp);
#endif
		
		outMessage->rndCallCount = lp->rng->count - rc;
		
		tw_event_send(synEvt);
		
		
	}else {
		id_type axonID = M->axonID;
		if ( s->connectionGrid[axonID][M->synapseCount] ) {
			id_type destinationNeuron = getNeuronGlobal(s->myCore, M->synapseCount);
			synEvt = tw_event_new(destinationNeuron, getNextEventTime(lp), lp);
			outMessage = (messageData *) tw_event_data(synEvt);
			outMessage->eventType = SYNAPSE_OUT;
			outMessage->axonID = M->axonID;
			
#ifdef SAVE_MSGS
			saveSynapseMessage(M, lp);
#endif
			outMessage->rndCallCount = lp->rng->count - rc;
			
			tw_event_send(synEvt);
		}
		if(M->synapseCount < NEURONS_IN_CORE){
			rc = lp->rng->count;
			synEvt = tw_event_new(lp->gid, getNextEventTime(lp), lp);
			outMessage = (messageData * ) tw_event_data(synEvt);
			outMessage->eventType = SYNAPSE_HEARTBEAT;
			outMessage->synapseCount = M->synapseCount + 1;
			outMessage->axonID = M->axonID;
#ifdef SAVE_MSGS
			saveSynapseMessage(M, lp);
#endif
			outMessage->rndCallCount = lp->rng->count - rc;
			tw_event_send(synEvt);
		}
		
		
	}
	

	
	
}
void synapse_reverse(synapseState *s, tw_bf *bf, messageData *M, tw_lp *lp){
	s->msgSent --;

	unsigned long count = M->rndCallCount;
	while (count --){
		tw_rand_reverse_unif(lp->rng);
	}
}

void synapse_final(synapseState *s, tw_lp *lp){
	//do some stats here if needed.
}

/** Neew Super Synapse with fanout code
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
