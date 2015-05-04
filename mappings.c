/*
 * mappings.c -- LP mapping functions were abstracted out of main in order to allow for easier testing.
 */
#include "mappings.h"


/* shadowed variables from the main code. Imp;emented this way to allow for testing and model sanity checking
 * inital values are used for tests. Init() function sets up the values from the main simulation.
 * */


	//TODO : Neurons are firing at other neurons.

//******************Mapping functions***********************//
void getLocalIDs(tw_lpid global, regid_t* core, regid_t* local) {
	(*core) = 0;
	(*local) = 0;
		//TODO: Check bit-shifting logic AGAIN
		//(*core) = CORE(global);
		//(*local) = LOC(global);
	(*core) = ((regid_t *) &global)[1] ;
	(*local) =  ((regid_t *)&global)[0];
	if(*local > CORE_SIZE * CORES_PER_PE){
		printf("ERROR\n\n\n\n");
		exit(-1);
	}
}
tw_lpid globalID(regid_t core, regid_t local) {
	tw_lpid returnVal = 0; //(tw_lpid)calloc(sizeof(tw_lpid) ,1);

	//returnVal = (tw_lpid)core << 32 | (tw_lpid) local;
	((int32_t *) &returnVal)[0] = local;
	((int32_t *) &returnVal)[1] = core;
	if(returnVal == 6442450954) {
		printf("ERROR2 \n");
		exit(-1);
	}
	return returnVal;
}

tw_lp* mapping_to_local(tw_lpid global) {
	regid_t core = 0;
	regid_t local = 0;
	getLocalIDs(global, &core, &local);
	tw_lpid id =  local + (core * CORE_SIZE);
	return tw_getlp(id);
}
	//TODO: Enance the speed of these by switching to macro!
regid_t getSynapseID(tw_lpid gid){
	regid_t local = ((regid_t*) &gid)[0];//LOC(gid);
	return local - NEURONS_IN_CORE;
}

regid_t getNeuronID(tw_lpid gid) {
	regid_t local = ((regid_t*) &gid)[0];
		//regid_t local = LOC(gid);
	return local ;
}

void getTypeLocalIDs(tw_lpid global, regid_t* core, regid_t* local){
	getLocalIDs(global, core, local);
		//determine type:
	regid_t ttl = CORE_SIZE * *core;
		if(*local - ttl > 0)
			*local =*local - ttl;

}

tw_peid mapping(tw_lpid gid) {
	regid_t core, local;
	getLocalIDs(gid, &core, &local);
		//core = CORE(gid);
	//local = LOC(gid);
	//the core is == to kp here.
	int rank = g_tw_mynode;
	tw_peid ccd = core / CORES_PER_PE;
	return ccd;
}


void initial_mapping(void ) {    tw_pe   *pe;
	int  nlp_per_kp;
	int  lpid;
	int  kpid;
	int  i;
	int  j;
	regid_t local = 0;
	int x = CORE_SIZE;
	int y = CORES_PER_PE;
		// may end up wasting last KP, but guaranteed each KP has == nLPs
	nlp_per_kp = (int)ceil((double) g_tw_nlp / (double) g_tw_nkp);

	if(!nlp_per_kp) {
		tw_error(TW_LOC, "Not enough KPs defined: %d", g_tw_nkp);
	}

	g_tw_lp_offset = g_tw_mynode * g_tw_nlp;

#if VERIFY_MAPPING
	printf("NODE %d: nlp %lld, offset %lld\n", g_tw_mynode, g_tw_nlp, g_tw_lp_offset);
#endif

	for(kpid = 0, lpid = 0, pe = NULL; (pe = tw_pe_next(pe)); ) {
#if VERIFY_MAPPING
		printf("\tPE %d\n", pe->id);
#endif


		for(i = 0; i < nkp_per_pe; i++, kpid++) {
			tw_kp_onpe(kpid, pe);

#if VERIFY_MAPPING
			printf("\t\tKP %d", kpid);
#endif


			for(j = 0; j < nlp_per_kp && lpid < g_tw_nlp; j++, lpid++) {
					//calculate core:
				regid_t core =  lpid / CORE_SIZE;
				tw_lp_onpe(lpid, pe, globalID(core,lpid));//g_tw_lp_offset+lpid);
				tw_lp_onkp(g_tw_lp[lpid], g_tw_kp[kpid]);

#if VERIFY_MAPPING
				if(0 == j % 20) {
					printf("\n\t\t\t");
				}
				printf("%lld ", lpid+g_tw_lp_offset);
#endif
			}



#if VERIFY_MAPPING
				if(0 == j % 20) {
					printf("\n\t\t\t");
				}
				printf("%lld ", lpid+g_tw_lp_offset);
#endif
			}

#if VERIFY_MAPPING
			printf("\n");
#endif

	}


	if(!g_tw_lp[g_tw_nlp-1]) {
		tw_error(TW_LOC, "Not all LPs defined! (g_tw_nlp=%d)", g_tw_nlp);
	}


}
/*
void initial_mapping_old(void) {
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
*/

