//
//  axon.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#include "axon.h"

void axon_init(axonState *s, tw_lp *lp)
{
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

        crTonicBurstingAxon(s, lp);
        specAxons ++;

    }else if(PHASIC_BURST_VAL){
        crTonicBurstingAxon(s, lp);
        specAxons ++;
    }
    else {
        s->sendMsgCount = 0;
        s->axonID = getAxonLocal(lp->gid);
        s->destSynapse = getSynapseFromAxon(lp->gid);
        // tw_stime r = getNextEventTime(lp);
        // tw_event *axe = tw_event_new(lp->gid, r, lp);
        // Msg_Data *data = (Msg_Data *)tw_event_data(axe);
        // data->eventType = AXON_OUT;
        // data->axonID = s->axonID;
        // tw_event_send(axe);

       
    }



    if (DEBUG_MODE) {

        printf("Axon type - %s, #%llu checking in with dest synapse %llu\n",s->axtype, lp->gid, s->destSynapse);
    }
    //printf("message ready at %f",r);
}