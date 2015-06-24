//
//  mapping.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#include "mapping.h"



long getCoreFromGID(tw_lpid gid)
{
	return CORE(gid);

}
long getCoreFromPE(tw_peid gid)
{
  return 0;
}


long getCoreLocalFromGID(tw_lpid gid)
{
	return LOCAL(gid);
}


tw_lpid globalID(id_t core, uint16_t i, uint16_t j)
{

   tw_lpid returnVal = 0; //(tw_lpid)calloc(sizeof(tw_lpid) ,1);
   // returnVal = (tw_lpid)core << 32 | (tw_lpid) local;
	uint32_t local = 0;
	((uint16_t*)&local)[0] = i;
	((uint16_t*)&local)[1] = j;
   ((uint32_t*)&returnVal)[lc] = local;
   ((uint32_t*)&returnVal)[co] = core;
   return returnVal;

}


tw_lpid getGlobalFromID(_idT core, _idT local){
	return  0; //globalID(core, LCID(local), LCID(local));
}//neurons are stored at i = 256, j 1-256;
tw_lpid getNeuronGlobal(_idT core, uint16_t neuron)
{
	return globalID(core, NEURONS_IN_CORE, neuron);
}

tw_lpid getAxonGlobal(_idT core, uint16_t axon)
{

	return globalID(core, axon, 0);
}

/**
 *  @detail  Since these are in i,j, with a NEURONS_IN_CORE dimensionality, just
 *	increment the synapse value j, and if it is > NEURONS_IN_CORE, then abort
 *
 */
tw_lpid getSynapseFromSynapse(tw_lpid synapse){
	_idT local = LOCAL(synapse);
	uint16_t j = JSIDE(local);
	j++;
	if(j > NEURONS_IN_CORE)
		abort();
	return globalID(CORE(synapse), ISIDE(local), j);

}
tw_lpid getNeuronFromSynapse(tw_lpid synapse){
	_idT local = LOCAL(synapse);
	uint16_t j = JSIDE(local);

	return globalID(CORE(synapse), AXONS_IN_CORE, j);

}
tw_lpid getSynapseFromAxon(tw_lpid axon){
	_idT local = LOCAL(axon);
	return globalID(CORE(axon), ISIDE(local), 1);
}
/**
 *  @details Since grid mapping works on PCS, here this is grid mapping with neurons.
 *	We have an X,Y,Z setup, where Z is a core. There may be 1 or more cores running
 *	on a PE, but generally there should be < 1 core per PE due to memory limitations.
 */
void tn_cube_mapping(){
	uint16_t x , y;
	tw_lpid lpid, kpid;
	tw_lpid numElementsPerKP, vpPerProc;
	int	numSynPerPE, numNerPerPe, numAxPerPE;
//
//	uint16_t myPEi, myPEj;
//	_idT myPECore;
//
//	tw_lpid local_lp_count;
//	tw_lpid local_kp_count;
//
//	numElementsPerKP = (CORE_SIZE * CORES_IN_SIM) / (NUM_VP_X * NUM_VP_Y);
//	VP_PER_PROC = (NUM_VP_X * NUM_VP_Y) / ((tw_nnodes() * g_tw_npe));
//		//Set the lp sizes and kp sizes:
//	g_tw_nlp = (CORE_SIZE * CORES_IN_SIM) / (tw_nnodes() * g_tw_nlp);
//	g_tw_nkp = VP_PER_PROC;

		//where do we start generating GIDs?
		//myPECore = peToCoreMap(g_tw_mynode);
		//grid mapping adapted for use with neural network model - here we have
		//x,y,z - with Z being a core.
	


}
void scatterMap(){
	int simSize = CORE_SIZE * CORES_IN_SIM;
	int LPsPerPE = simSize / tw_nnodes();

		//Create the 1-D table
	tw_lpid *gidArray = NULL;
	tw_lpid *gePEMap = (tw_lpid*) calloc(sizeof(tw_lpid), tw_nnodes());

	if(g_tw_mynode == 0) {
		gidArray = (tw_lpid*) calloc(sizeof(tw_lpid), simSize);
			//Create mappings:
		uint16_t i = AXONS_IN_CORE;
		uint16_t j = NEURONS_IN_CORE;
		uint32_t c = CORES_IN_SIM;
		int actual = 0;
		for (uint32_t z = 0; z < c; z ++) {
			for (uint16_t x = 0; x <= j; x ++) {
					//x and j are horizontal.
				for(uint16_t y = 0; y <= i; y ++){
					if(y > 0 || x != NEURONS_IN_CORE)
						{
						gidArray[actual] = globalID(z, y, x);
						actual ++;
						}
				}
			}
		}

	}
	printf("\n\n\ntotal lps %i", simSize);
	//create GID arrays for each LP:
	MPI_Barrier(MPI_COMM_WORLD);
	myGIDs = (tw_lpid *)malloc(sizeof(tw_lpid)* LPsPerPE);
	MPI_Scatter(gidArray, LPsPerPE, MPI_UINT64_T, myGIDs, LPsPerPE, MPI_UINT64_T, 0, MPI_COMM_WORLD);
		//now all GIDs have been seeded to the sim.

	int ct = 0;
	while (ct <= tw_nnodes() ){

		MPI_Barrier(MPI_COMM_WORLD);
	if(g_tw_mynode == ct && simSize < 1024) {
		printf("LPs/PE %i", LPsPerPE);
		printf("PEID #%li HAS \n CORE\ti\tj\n",g_tw_mynode);
		int i;

		for( i = 0; i < LPsPerPE; i ++) {
			int32_t loc = LOCAL(myGIDs[i]);
			printf("%i\t%i\t%i\n",CORE(myGIDs[i]), ISIDE(loc), JSIDE(loc));
		}
		for( i = 0; i < LPsPerPE; i ++)
			printf("%llu\n", myGIDs[i]);
		printf("for a total of %i elements\n\n", i);

	}

		ct++;
	}
	if(g_tw_mynode == 0){ //free the memory after doing things
		free(gidArray);
	}

}


tw_peid lpToPeMap(tw_lpid gid) {
		//Given a gid return a PE.
	id_t core = CORE(gid);
	id_t loc = LOCAL(gid);
	uint16_t i = ISIDE(loc);
	uint16_t j = JSIDE(loc);
		//i and j are limited by the number of axons and neurons.
	
}