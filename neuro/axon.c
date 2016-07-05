//
//  axon.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#include "axon.h"

//Global Message CSV Writer -- for debug and message traacing
#ifdef SAVE_MSGS
csv_writer * messageTrace;

void axon_mark_message(axonState *s, messageData *M, tw_lpid gid, tw_lp *lp){

	if(!M->originGID){
		M->originGID = gid;
		M->msgCreationTime = tw_now(lp);
		M->idp1 = s->axonID;
		M->idp2 = getCoreFromGID(gid);
		M->idp3 = s->sendMsgCount;
	}
	char * dm = tw_calloc(TW_LOC,2, sizeof(char), 256);
	print(M->uuid);printf("\n");
	sprint(dm,M->uuid);
	addCol(messageTrace,dm , 0);
	dm = tw_calloc(TW_LOC,2, sizeof(char), 256);
	sprint(dm,M->originGID);
	addCol(messageTrace, dm ,0);
	dm = tw_calloc(TW_LOC,2 ,sizeof(char), 256);
	sprint(dm,M->msgCreationTime);
	addCol(messageTrace, dm, 0);
	dm = tw_calloc(TW_LOC,2,  sizeof(char), 256);
	//sprint(dm,"Axon");
	sprintf(dm, "%s", "Axon");
	addCol(messageTrace, dm, 1);
	addCol(messageTrace, lp->gid, 0);
	addRow(messageTrace);
	
}
#endif
void axon_init(axonState *s, tw_lp *lp)
{
	static bool writeInit = false;
    //TODO: Maybe switch this to a switch/case later, since it's going to get
    //big.
	static int specAxons = 0;
    s->axtype = "NORM";
    if(PHAS_VAL) {//one phasic axon:
        if (specAxons == 0){
            //crPhasicAxon(s, lp);
            specAxons ++;
        }

        else {
            s->sendMsgCount = 0;
            s->axonID = (lp->gid);
            s->destSynapse = getSynapseFromAxon(lp->gid);
        }

    }else if(TONIC_BURST_VAL){

        //crTonicBurstingAxon(s, lp);
        specAxons ++;
		printf("Tonic bursting validation not available in this version of NeMo\n");

    }else if(PHASIC_BURST_VAL){
        //crTonicBurstingAxon(s, lp);
		printf("Phasic bursting validation not available in this version of NeMo\n");
        specAxons ++;
    }else if(FILE_IN){
        //Do file processing - load in the initial spikes here.
        if (g_tw_mynode == 0){
            tw_printf(TW_LOC,"Axon ID %llu attempted file load.\n",s->axonID);
        }
    }
    else { //else this is a random network for benchmarking.
        s->sendMsgCount = 0;
        s->axonID = getAxonLocal(lp->gid);
        s->destSynapse = getSynapseFromAxon(lp->gid);
        tw_stime r = getNextBigTick(lp,0);
        tw_event *axe = tw_event_new(lp->gid, r, lp);
        messageData *data = (messageData *)tw_event_data(axe);
        data->eventType = AXON_OUT;
        data->axonID = s->axonID;
		tw_event_send(axe);
		if (SAVE_MSGS){
			if(!writeInit){
				messageTrace = createCSV("message_log", g_tw_mynode, g_tw_npe -1);
				writeInit = true;
				
			}
			
			axon_mark_message(s, data, lp->gid, lp);
			/** @todo add message save code to io stack
			 * */
			// save message //
			
		}
       
    }



    if (DEBUG_MODE) {

        printf("Axon type - %s, #%llu checking in with dest synapse %llu\n",s->axtype, lp->gid, s->destSynapse);
    }


    //printf("message ready at %f",r);
}


void axon_event(axonState *s, tw_bf *CV, messageData *M, tw_lp *lp){
	//generate new message
	enum evtType mt = M->eventType;
	long rc = lp->rng->count;
	tw_event *axonEvent = tw_event_new(s->destSynapse, getNextEventTime(lp), lp);
	messageData *data = (messageData *) tw_event_data(axonEvent);

	data->localID = lp->gid;
	data->eventType = AXON_OUT;
	data->axonID = s->axonID;

	tw_event_send(axonEvent);

	//End message generation - add message count and check random times
	s->sendMsgCount ++;
	M->rndCallCount = lp->rng->count - rc;

	if (VALIDATION){
		//save axon event for validation and traceback. 
	}
}
void axon_reverse(axonState *s, tw_bf *CV, messageData *M, tw_lp *lp){
	if(VALIDATION) {
		//Undo save axon event for validation
	}

	long count  = M->rndCallCount;
	while (count--){
		tw_rand_reverse_unif(lp->rng);
	}


}
void axon_final(axonState *s, tw_lp *lp){
	writeCSV(messageTrace);
	closeCSV(messageTrace);
}