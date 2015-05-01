/*
 * mappings.c -- LP mapping functions were abstracted out of main in order to allow for easier testing.
 */
#include "mappings.h"


/* shadowed variables from the main code. Imp;emented this way to allow for testing and model sanity checking
 * inital values are used for tests. Init() function sets up the values from the main simulation.
 * */
int coresPerPe  = 4;
int synapsesInCore= 256;
int neuronsInCore = 128;
extern tw_lptype model_lps[];



void initMapVars(int nInCore,int sInCore,int cpe) {
	coresPerPe = cpe;
	synapsesInCore = sInCore;
	neuronsInCore = nInCore;

}

//******************Mapping functions***********************//
void getLocalIDs(tw_lpid global, regid_t* core, regid_t* local) {
	(*core) = CORE(global);
	(*local) = LOC(global);
}
tw_lpid globalID(regid_t core, regid_t local) {
	tw_lpid returnVal = 0; //(tw_lpid)calloc(sizeof(tw_lpid) ,1);

	//returnVal = (tw_lpid)core << 32 | (tw_lpid) local;
	((int32_t *) &returnVal)[0] = local;
	((int32_t *) &returnVal)[1] = core;
	return returnVal;
}
tw_lp* mapping_to_local(tw_lpid global) {
	regid_t core;
	regid_t local;
	getLocalIDs(global, &core, &local);
	tw_lpid id = (core * CORE_SIZE) + local;
	return tw_getlp(id);
}

tw_peid mapping(tw_lpid gid) {
	regid_t core, local;
	//getLocalIDs(gid, &core, &local);
	core = CORE(gid);
	//local = LOC(gid);
	//the core is == to kp here.
	int rank = g_tw_mynode;
	tw_peid ccd = core / CORES_PER_PE;
	return ccd;
}


void initial_mapping(void) {
	tw_pe* pe;
	// Check that we don't want more KP per LP.
	int j = 0;
	for (tw_lpid kpid = 0; kpid < coresPerPe; kpid++) {

		tw_kp_onpe(kpid, g_tw_pe[0]);  // kp on this pe - each core is a KP.
		// Now define the neurons/synapses running on this core/kp

		for (int i = 0; i < neuronsInCore; i++) {  // create the neurons first

			// Right now, we are only allowing one pe per process.
			// pe = tw_getpe(kpid % g_tw_npe);
			pe = tw_getpe(0);
			tw_clock_setup();
			regid_t myCore = kpid + g_tw_mynode;
			regid_t myLocal = j;
			tw_lpid id = globalID(myCore, myLocal);
			// Create a new LP with the bit-twiddled gid
			tw_lp_onpe(j, pe, id);
			tw_lp_onkp(tw_getlp(j), g_tw_kp[kpid]);
			tw_lp_settype(j, &model_lps[0]);

			if (DEBUG_MODE)
				printf("Neuron created on LP %lu, core:local %lu:%lu, kp is %lu",
				       pe->id, myCore, myLocal, kpid);
			j++;
		}
		// create synapses
		for (int i = 0; i < synapsesInCore; i++) {
			pe = tw_getpe(0); //todo - add support for more PEs
			regid_t myCore = kpid + g_tw_mynode;
			regid_t myLocal = j;
			tw_lpid id = globalID(myCore, myLocal);
			// Create a new LP with the bit-twiddled gid
			tw_lp_onpe(j, pe, id);
			tw_lp_onkp(tw_getlp(j), g_tw_kp[kpid]);
			tw_lp_settype(j, &model_lps[1]);
			if (DEBUG_MODE)
				printf("Neuron created on LP %lu, core:local %u:%lu, kp is %lu", pe->id,
				       myCore, myLocal, kpid);
			j++;
		}
	}
}


