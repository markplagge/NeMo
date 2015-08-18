//
//  axon.h
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#ifndef __ROSS_TOP__axon__
#define __ROSS_TOP__axon__

#include <stdio.h>
#include "../assist.h"
#include "../input_simulator.h"

typedef struct AxonState {
	stat_type sendMsgCount;
	tw_lpid destSynapse;
	id_type axonID;

	inputSimulatorState *sim;
}axonState;
/**
 *  @brief  Handles a message sent to an Axon.
 *
 *  @param st state
 *  @param M  message
 *  @param lp lp
 */
void axonReceiveMessage(axonState *st, Msg_Data *M, tw_lp *lp);
void axonReverseState(axonState *st, Msg_Data *M, tw_lp *lp);
#endif /* defined(__ROSS_TOP__axon__) */
