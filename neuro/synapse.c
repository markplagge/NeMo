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

void sendSynapseHB(synapseState *s, tw_bf *bf, messageData *M, tw_lp *lp, unsigned long count){
	tw_event * synapseHB = tw_event_new(lp->gid, getNextSynapseHeartbeat(lp), lp);
	messageData * outData = tw_event_data(synapseHB);
	outData->synapseCounter = count - 1;
	outData->axonID = M->axonID;
	outData->localID = M->localID;
	outData->eventType = SYNAPSE_HEARTBEAT;
	
	tw_event_send(synapseHB);
	
}
void reverseSynapseHB(synapseState *s, tw_bf *bf, messageData *M, tw_lp *lp){
	M->synapseCounter ++;
}

void synapse_event(synapseState *s, tw_bf *bf, messageData *M, tw_lp *lp){
	unsigned long randCount = lp->rng->count;
	
	
	if(M->eventType == SYNAPSE_HEARTBEAT){
		//Heartbeat message
		if(M->synapseCounter != 0){
            //unsigned long sc = M->synapseCounter - 1;
            sendSynapseHB(s, bf, M, lp, M->synapseCounter);
        }
        
		tw_lpid neuron = getNeuronGlobal(s->myCore, M->synapseCounter);
		tw_event * sout = tw_event_new(neuron, getNextEventTime(lp),lp);
		messageData * outData = tw_event_data(sout);
		outData->axonID = M->axonID;
		outData->localID = M->axonID;
		outData->eventType = SYNAPSE_OUT;
        tw_event_send(sout);
        ++ s->msgSent;
		
	}else if(M->eventType == AXON_OUT){
		sendSynapseHB(s, bf, M, lp, NEURONS_IN_CORE);
	}
	
	M->rndCallCount = lp->rng->count - randCount;
	
}
//	static int hasRun = 0;
//	
//	if (! hasRun) {
//		for (int i = 0; i < NEURONS_IN_CORE; i++){
//			for(int j = 0; j < NEURONS_IN_CORE; j ++){
//				s->connectionGrid[i][j] = tw_getlp(i)->cur_state->synapticConnectivity[j];
//			}
//		}
//	}
//	
//	long rc = lp->rng->count;
//	//run a loop that calls the "forward event handler" of each neuron in this core:
//	tw_lp * cNeuron;
//	
//	/** @TODO: See if localID is still needed ! */
//	
//	//Create a "message" that is "sent" to this neuron
//	messageData *outM = (messageData *) tw_calloc(TW_LOC, "Synapse", sizeof(messageData), 1);
//	//set up the message for the neurons
//	outM->axonID = M->axonID;
//	outM->eventType = SYNAPSE_OUT;
//	outM->localID = M->axonID;
//	
//	
//	id_type axonID = M->axonID;
//	for(int i = 0; i < AXONS_IN_CORE; i ++){
//		//check to see if the neuron is connected to the axon that sent the message
//		//get the neuron GID
//		tw_lpid nid = getNeuronGlobal(s->myCore,i);
//		//get the LP @todo: look at changing this to direct array access
//		cNeuron = tw_getlp(nid);
//		
//		
//		
//		//if(cNeuron->connectionGrid[axonID] != 0){
//			
//
//
//			//call the neuron's forward event handler
//			/** @todo: This is a bandaid until proper reverse computation can be determined. */
//			cNeuron->type->event(cNeuron->cur_state,&s->neuronBF[i],outM,cNeuron);
//			s->randCount[i] = outM->rndCallCount;
//			
//		//}
//
//	}


///** TODO: This is probably not going to work. I think the synapse will need a larger state. */
//
//
//		messageData *outMessage;
//		tw_event *synEvt;
//
//	if (M->eventType == AXON_OUT:){
//		synEvt = tw_event_new(lp->gid, getNextEventTime(lp),lp );
//		outMessage = (messageData *) tw_event_data(synEvt);
//		outMessage->eventType = SYNAPSE_HEARTBEAT;
//		outMessage->synapseCount = 0;
//		outMessage->axonID = M->axonID;
//#ifdef SAVE_MSGS
//		saveSynapseMessage(M, lp);
//#endif
//
//		outMessage->rndCallCount = lp->rng->count - rc;
//
//		tw_event_send(synEvt);
//
//
//	}else {
//		id_type axonID = M->axonID;
//		if ( s->connectionGrid[axonID][M->synapseCount] ) {
//			id_type destinationNeuron = getNeuronGlobal(s->myCore, M->synapseCount);
//			synEvt = tw_event_new(destinationNeuron, getNextEventTime(lp), lp);
//			outMessage = (messageData *) tw_event_data(synEvt);
//			outMessage->eventType = SYNAPSE_OUT;
//			outMessage->axonID = M->axonID;
//
//#ifdef SAVE_MSGS
//			saveSynapseMessage(M, lp);
//#endif
//			outMessage->rndCallCount = lp->rng->count - rc;
//
//			tw_event_send(synEvt);
//		}
//		if(M->synapseCount < NEURONS_IN_CORE){
//			rc = lp->rng->count;
//			synEvt = tw_event_new(lp->gid, getNextEventTime(lp), lp);
//			outMessage = (messageData * ) tw_event_data(synEvt);
//			outMessage->eventType = SYNAPSE_HEARTBEAT;
//			outMessage->synapseCount = M->synapseCount + 1;
//			outMessage->axonID = M->axonID;
//#ifdef SAVE_MSGS
//			saveSynapseMessage(M, lp);
//#endif
//			outMessage->rndCallCount = lp->rng->count - rc;
//			tw_event_send(synEvt);
//		}
//
//
//	}
//

	
	
	//}
void synapse_reverse(synapseState *s, tw_bf *bf, messageData *M, tw_lp *lp){
    
    
    if(M->eventType == AXON_OUT){
        
    }else if(M->eventType == SYNAPSE_HEARTBEAT){
        -- s->msgSent;
    }
	unsigned long count = M->rndCallCount;
	while (count --){
		tw_rand_reverse_unif(lp->rng);
	}
}

void synapse_final(synapseState *s, tw_lp *lp){
	//do some stats here if needed.
    
    if(g_tw_synchronization_protocol == OPTIMISTIC_DEBUG) {
        char * shdr = "Synapse Error\n";
        
        if (s->msgSent != 0){
            print(shdr);
            debugMsg("Message Sent Val ->", s->msgSent);
        }
    }

}

