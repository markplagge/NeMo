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
typedef struct AxonState {
	_statT recvMsgCount;
	_statT sendMsgCount;
	tw_lpid destSynapse;
}axonState;

#endif /* defined(__ROSS_TOP__axon__) */
