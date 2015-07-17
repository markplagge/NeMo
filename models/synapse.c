//
//  synapse.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#include "synapse.h"

void synapseReceiveMessage(synapseState *st, Msg_Data *M, tw_lp *lp)
{
	long start_count = lp->rng->count;
	tw_stime time;

	st->msgSent++;

	time = getNextEventTime(lp);
	tw_event *newEvent = tw_event_new(st->destNeuron, time, lp);
	Msg_Data *data = (Msg_Data * )tw_event_data(newEvent);
	data->eventType = SYNAPSE_OUT;
	tw_event_send(newEvent);

	//check to see if this is the last synapse:
	if (st->mySynapseNum < NEURONS_IN_CORE) {
		time = getNextEventTime(lp);
		tw_event *newEvent = tw_event_new(st->destSynapse, time, lp);
		Msg_Data *data = (Msg_Data * )tw_event_data(newEvent);
		data->eventType = SYNAPSE_OUT;
		tw_event_send(newEvent);
		st->msgSent++;
	}



	M->rndCallCount = lp->rng->count - start_count;
}


void synapseReverseState(synapseState *st, Msg_Data *M, tw_lp *lp)
{
	if (st->mySynapseNum < NEURONS_IN_CORE) {
		st->msgSent--;
	}
	long count = M->rndCallCount;
	while (count--)
	{
		tw_rand_reverse_unif(lp->rng);
	}
}
