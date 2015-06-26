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


_gridIDT iSizeOffset(){
	return NEURONS_IN_CORE;
}
_gridIDT jSizeOffset(){
	return AXONS_IN_CORE;

}

_gridIDT  jVal(tw_lpid gid){
	uint32_t lo = LOCAL(gid);
	return (JSIDE(lo)) * jSizeOffset();
}
_gridIDT iVal(tw_lpid gid) {
	uint32_t lo = LOCAL(gid);
	return (ISIDE(lo));
}
_gridIDT cVal(tw_lpid gid) {
	uint32_t core = CORE(gid);
	return core * (CORE_SIZE + 1 );

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
	return globalID(core, ISIDE(local), JSIDE(local));
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
	_gridIDT j = JSIDE(local);
	j++;
	if(j > NEURONS_IN_CORE)
		abort();
	return globalID(CORE(synapse), ISIDE(local), j);

}
tw_lpid getNeuronFromSynapse(tw_lpid synapse){
	_idT local = LOCAL(synapse);
	_gridIDT j = JSIDE(local);

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
bool dontSkip(tw_lpid gid){
	_idT loc = LOCAL(gid);
	_idT jside = JSIDE(loc);
	_idT iside = ISIDE(loc);
	if(JSIDE(loc) == 0 && ISIDE(loc) == (NEURONS_IN_CORE - 1))
		return false;
	return true;
}
void scatterMap(){

		//int LPsPerPE = SIM_SIZE / tw_nnodes();

		//Create the 1-D table
	tw_lpid *gidArray = NULL;

	gidArray = (tw_lpid*) calloc(sizeof(tw_lpid),SIM_SIZE);
			//Create mappings:

			//use modulus and a single loop for this
	int i;
	for(i = 0; i < LPS_PER_PE; i ++ ) {

		tw_lpid id = localToGlobal(i);
		if(dontSkip(id)){
			gidArray[i] = id;
		}
		else{
			printf("\nskipped\n");
		}
	}


	printf("\n\n\ntotal lps %i - actual is %i", SIM_SIZE, i);
	//create GID arrays for each LP:

	myGIDs = (tw_lpid *)malloc(sizeof(tw_lpid)* LPS_PER_PE);
		//wait for alloc to happen.
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Scatter(gidArray, LPS_PER_PE, MPI_UINT64_T, myGIDs, LPS_PER_PE, MPI_UINT64_T, 0, MPI_COMM_WORLD);
		//now all GIDs have been seeded to the sim.

	int ct = 0;

	while (ct <= tw_nnodes() ){

		MPI_Barrier(MPI_COMM_WORLD);
	if(g_tw_mynode == ct && SIM_SIZE < 1024) {
		printf("LPs/PE %i", LPS_PER_PE);
		printf("PEID #%li HAS \n CORE\ti\tj\tlocal\n",g_tw_mynode);
		int i;

		for( i = 0; i < LPS_PER_PE; i ++) {

			_idT loc = LOCAL(myGIDs[i]);
			tw_peid pe = getPEFromGID(myGIDs[i]);;

			printf("\t%i\t%i\t%i\t%llu\n",CORE(myGIDs[i]), ISIDE(loc), JSIDE(loc),globalToLocalID(myGIDs[i]));
			if(pe != g_tw_mynode)
				printf("ERROR - calced PE %i is not my PE %i \n", pe, g_tw_mynode);


		}
			printf("for a total of %i elements\n\n", i);

	}

		ct++;
	}


	if(g_tw_mynode == 0){ //free the memory after doing things
		free(gidArray);
	}


}


tw_peid getPEFromGID(tw_lpid gid) {
		//Given a gid return a PE.
	_idT cc = combVal(gid); ///@todo why does this go to one after dividing?
	_idT res = cc / LPS_PER_PE;

	return (tw_peid)res;
		//i and j are limited by the number of axons and neurons.
	
}

tw_lp* globalToLP(tw_lpid gid){

	return g_tw_lp[globalToLocalID(gid)];
}
tw_lpid globalToLocalID(tw_lpid gid) {
	_idT cc = combVal(gid);
	tw_lpid peoff = g_tw_mynode *  LPS_PER_PE;
	return cc - peoff;


}
_idT combVal(tw_lpid gid){
	_idT i = iVal(gid);
	_idT c = cVal(gid);
	_idT j = jVal(gid);

	return i   + c + j;
}

tw_lpid localToGlobal(tw_lpid local){
	_gridIDT localJ = local / jSizeOffset();
	_gridIDT localI = local % iSizeOffset();
	_idT core = (localI * jSizeOffset() ) / CORE_SIZE;
	return globalID(core,localI, localJ);
}