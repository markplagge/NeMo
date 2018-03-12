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
//csv_writer * messageTrace;
//
//void axon_mark_message(axonState *s, messageData *M, tw_lpid gid, tw_lp *lp){
//
//	if(!M->originGID){
//		M->originGID = gid;
//		M->msgCreationTime = tw_now(lp);
//		M->idp1 = s->axonID;
//		M->idp2 = getCoreFromGID(gid);
//		M->idp3 = s->sendMsgCount;
//	}
//	char * dm = tw_calloc(TW_LOC,2, sizeof(char), 256);
//	print(M->uuid);printf("\n");
//	sprint(dm,M->uuid);
//	addCol(messageTrace,dm , 0);
//	dm = tw_calloc(TW_LOC,2, sizeof(char), 256);
//	sprint(dm,M->originGID);
//	addCol(messageTrace, dm ,0);
//	dm = tw_calloc(TW_LOC,2 ,sizeof(char), 256);
//	sprint(dm,M->msgCreationTime);
//	addCol(messageTrace, dm, 0);
//	dm = tw_calloc(TW_LOC,2,  sizeof(char), 256);
//	//sprint(dm,"Axon");
//	sprintf(dm, "%s", "Axon");
//	addCol(messageTrace, dm, 1);
//	addCol(messageTrace, lp->gid, 0);
//	addRow(messageTrace);
//	
//}
#endif
FILE *dumpi_out1;
void scheduleSpike(long time, id_type axonID, tw_lp *lp) {
  tw_stime sched_event = time + JITTER;
  tw_event *axevt = tw_event_new(lp->gid, sched_event, lp);
  messageData *data = tw_event_data(axevt);
  data->axonID = axonID;
  data->originGID = lp->gid;
  data->eventType = AXON_OUT;
  tw_event_send(axevt);
}
int axonct = 0;
void axonSpikeReader(axonState *s, tw_lp *lp) {
  static char announce = 1;
  //static int axonct = 0;
  if (g_tw_mynode==0) {
    if (announce) {
      printf("Starting axon loading of spikes.\n");
      announce = 0;
    }
    if (axonct%100==0) {
      printf("Axon %i - %i checking in.\n ", s->axonID, axonct);
      axonct++;
    }

  }
  testImport();
  list_t spikeList;
  list_init(&spikeList);
//    if(getCoreFromGID(lp->gid) > 511){
//        printf("high order here.\n");
//    }
  int numSpikes = getSpikesFromAxon(&spikeList, getCoreFromGID(lp->gid), s->axonID);
  if (numSpikes > 0) {
    list_iterator_start(&spikeList);
    while (list_iterator_hasnext(&spikeList)) {
      scheduleSpike(*(long *) list_iterator_next(&spikeList), s->axonID, lp);

    }
    list_iterator_stop(&spikeList);

  }
  spikeFromAxonComplete(&spikeList);

}

void axon_init(axonState *s, tw_lp *lp) {
  static int fileInit = 0;
  ///// DUMPI FILE
  if (DO_DUMPI && !fileInit) {
    char *fn = calloc(sizeof(char), 256);
    sprintf(fn, "dumpi_virt-%i_rnk%li-rcvr.txt", getCoreFromGID(lp->gid), g_tw_mynode);
    dumpi_out1 = fopen(fn, "w");
    free(fn);
    fileInit = 1;
  }

  static bool writeInit = false;
  //TODO: Maybe switch this to a switch/case later, since it's going to get
  //big.
  static int specAxons = 0;
  s->axtype = "NORM";

  if (FILE_IN) { //FILE_IN){

    s->sendMsgCount = 0;
    s->axonID = getAxonLocal(lp->gid);
    s->destSynapse = getSynapseFromAxon(lp->gid);
    //Moved this functionality to synapses.
    //axonSpikeReader(s,lp);

    specAxons++;

  } else if (PHAS_VAL) {//one phasic axon:
    if (specAxons==0) {
      //crPhasicAxon(s, lp);
      specAxons++;
    } else {
      s->sendMsgCount = 0;
      s->axonID = (lp->gid);
      s->destSynapse = getSynapseFromAxon(lp->gid);
    }

  } else if (TONIC_BURST_VAL) {

    //crTonicBurstingAxon(s, lp);
    specAxons++;
    printf("Tonic bursting validation not available in this version of NeMo\n");

  } else if (PHASIC_BURST_VAL) {
    //crTonicBurstingAxon(s, lp);
    printf("Phasic bursting validation not available in this version of NeMo\n");
    specAxons++;
//    }else if(FILE_IN){
//        //Do file processing - load in the initial spikes here.
//        if (g_tw_mynode == 0){
//			//why would an axon want to load a file?
//			//tw_printf(TW_LOC,"Axon ID %llu attempted file load.\n",s->axonID);
//			
//        }
//    }
  } else { //else this is a random network for benchmarking.
    s->sendMsgCount = 0;
    s->axonID = getAxonLocal(lp->gid);
    s->destSynapse = getSynapseFromAxon(lp->gid);
    tw_stime r = getNextBigTick(lp, 0);
    tw_event *axe = tw_event_new(lp->gid, r, lp);
    messageData *data = (messageData *) tw_event_data(axe);
    data->eventType = AXON_OUT;
    data->axonID = s->axonID;
    tw_event_send(axe);

  }

  if (DEBUG_MODE) {

    printf("Axon type - %s, #%llu checking in with dest synapse %llu\n", s->axtype, lp->gid, s->destSynapse);
  }


  //printf("message ready at %f",r);
}

void axon_event(axonState *s, tw_bf *CV, messageData *M, tw_lp *lp) {
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
  s->sendMsgCount++;
  M->rndCallCount = lp->rng->count - rc;

  if (VALIDATION) {
    //save axon event for validation and traceback.
  }

}
void axon_reverse(axonState *s, tw_bf *CV, messageData *M, tw_lp *lp) {
  if (VALIDATION) {
    //Undo save axon event for validation
  }
  --s->sendMsgCount;

  long count = M->rndCallCount;
  while (count--) {
    tw_rand_reverse_unif(lp->rng);
  }

}
void axon_final(axonState *s, tw_lp *lp) {
  static int fileOpen = 1;
  if (g_tw_synchronization_protocol==OPTIMISTIC_DEBUG) {
    char *shdr = "Axon Error\n";

    if (s->sendMsgCount!=0) {
      //print(shdr);
      char *m = "Message Sent Val ->";
      //debugMsg(m, s->sendMsgCount);
    }
  }

  if (DO_DUMPI && fileOpen) {
    fclose(dumpi_out1);
    fileOpen = 0;
  }

}
void axon_commit(axonState *s, tw_bf *CV, messageData *M, tw_lp *lp) {
  if (DO_DUMPI && M->isRemote) {
    //saveMPIMessage(s->myCoreID, getCoreFromGID(s->outputGID), tw_now(lp),
    //			   dumpi_out);
    setrnd(lp);
    saveRecvMessage(M->isRemote, getCoreFromGID(lp->gid), tw_now(lp), 0, dumpi_out1);
  }
}