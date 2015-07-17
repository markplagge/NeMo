//
//  axon.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#include "axon.h"

void axonReceiveMessage(axonState *st, Msg_Data *M, tw_lp *lp)
{
	long start_count = lp->rng->count;
	tw_stime time;

	st->sendMsgCount++;
	if (M->eventType == NEURON_OUT) {
		time = getNextEventTime(lp);
		tw_event *newEvent = tw_event_new(st->destSynapse, time, lp);
		Msg_Data *data = (Msg_Data * )tw_event_data(newEvent);
		data->eventType = AXON_OUT;
		tw_event_send(newEvent);
	}else  {
		//Call the signal generator. TBD
	}
	M->rndCallCount = lp->rng->count - start_count;
}


void axonReverseState(axonState *st, Msg_Data *M, tw_lp *lp)
{
	st->sendMsgCount--;
	long count = M->rndCallCount;
	while (count--)
	{
		tw_rand_reverse_unif(lp->rng);
	}
}
