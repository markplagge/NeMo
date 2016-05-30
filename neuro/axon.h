#ifndef __NEMO_axon__
#define __NEMO_axon__
#include "../globals.h"
typedef struct AxonState {
	stat_type sendMsgCount;
	tw_lpid destSynapse;
	id_type axonID;

	//inputSimulatorState *sim;

	char* axtype;
}axonState;

#endif
