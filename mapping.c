//
//  mapping.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#include "mapping.h"


/**{*/
//Linear Mapping functions

/**
 *  @brief  Linear mapping core gid offset
 *
 *  @param gid current GID
 *
 *  @return returns the number of lps before this core.
 */
tw_lpid lCoreOffset(tw_lpid gid)
{
	return (lGetCoreFromGID(gid) * CORE_SIZE);
}


tw_peid lGidToPE(tw_lpid gid)
{
	return ((tw_peid)gid / g_tw_nlp);
}


tw_lpid lGetSynFromAxon(tw_lpid axeGID)
{
	tw_lpid synAt = lCoreOffset(axeGID) + CORE_SIZE;

	synAt -= NEURONS_IN_CORE + SYNAPSES_IN_CORE;
	tw_lpid adj = axeGID % AXONS_IN_CORE;

	return (synAt + (AXONS_IN_CORE * adj));
	//return AXONS_IN_CORE + (axeGID * AXONS_IN_CORE);
}


tw_lpid lGetNextSynFromSyn(tw_lpid synGID)
{
	tw_lpid nextSyn = synGID + 1;

	if (nextSyn % (AXONS_IN_CORE)) {
		return (nextSyn);
	}
	return (0);
}


tw_lpid lGetNeuronFromSyn(tw_lpid synGID)
{
	tw_lpid neuAt = lCoreOffset(synGID) + CORE_SIZE;

	neuAt -= NEURONS_IN_CORE;
	tw_lpid adj = (synGID - lCoreOffset(synGID)) % NEURONS_IN_CORE;
	neuAt += (adj - 1) * 1;

	long rowID = (lCoreOffset(synGID) - AXONS_IN_CORE) / NEURONS_IN_CORE;

	//long offset = rowID * NEURONS_IN_CORE;


	return (neuAt + 1);
}


tw_lpid lGetAxonFromNeu(id_type core, id_type axeNum)
{
	tw_lpid coreOff = core * CORE_SIZE;

	return (axeNum + coreOff);
}


tw_lpid lGetSynNumLocal(tw_lpid gid)
{
	return (gid - lCoreOffset(gid) - AXONS_IN_CORE);
}


tw_lpid lGetAxeNumLocal(tw_lpid gid)
{
	return (gid - lCoreOffset(gid));
}


tw_lpid lGetNeuNumLocal(tw_lpid gid)
{
	//return (gid - lCoreOffset(gid));
	tw_lpid neuronStart = (SYNAPSES_IN_CORE + AXONS_IN_CORE);
	return gid-lCoreOffset(gid) - neuronStart;
}


/**}*/


//Cube and custom linear mapping ************************


long getCoreFromGID(tw_lpid gid)
{
	if (tnMapping == CUST_LINEAR) {
		return (CORE(gid));
	} else if (tnMapping == LLINEAR) {
		return (gid / CORE_SIZE);
	}
	return (0);
}


long getCoreLocalFromGID(tw_lpid gid)
{
	if (tnMapping == CUST_LINEAR) {
		return (LOCAL(gid));
	} else {
		tw_lpid off = lCoreOffset(gid);
		return (gid - off);
	}
}


_gridIDT iSizeOffset()
{
	return (NEURONS_IN_CORE);
}


_gridIDT jSizeOffset()
{
	return (AXONS_IN_CORE);
}


_gridIDT jVal(tw_lpid gid)
{
	uint32_t lo = LOCAL(gid);

	return ((JSIDE(lo)) * jSizeOffset());
}


_gridIDT iVal(tw_lpid gid)
{
	uint32_t lo = LOCAL(gid);

	return (ISIDE(lo));
}


_gridIDT cVal(tw_lpid gid)
{
	uint32_t core = CORE(gid);

	return (core * (CORE_SIZE + 1));
}


tw_lpid lGetCoreFromGID(tw_lpid gid)
{
	return (gid / CORE_SIZE);
}


tw_lpid nodeOffset()
{
	return (LPS_PER_PE * g_tw_mynode);
}


tw_lpid globalID(id_type core, uint16_t i, uint16_t j)
{
	tw_lpid returnVal = 0; //(tw_lpid)calloc(sizeof(tw_lpid) ,1);
	// returnVal = (tw_lpid)core << 32 | (tw_lpid) local;
	uint32_t local = 0;

	((uint16_t *)&local)[0] = i;
	((uint16_t *)&local)[1] = j;
	((uint32_t *)&returnVal)[lc] = local;
	((uint32_t *)&returnVal)[co] = core;
	return (returnVal);
}


tw_lpid getGlobalFromID(id_type core, id_type local)
{
	return (globalID(core, ISIDE(local), JSIDE(local)));
}


//neurons are stored at i = 256, j 1-256;
tw_lpid getNeuronGlobal(id_type core, uint16_t neuron)
{
	return (globalID(core, NEURONS_IN_CORE, neuron));
}


tw_lpid getAxonGlobal(id_type core, uint16_t axon)
{
	return (globalID(core, axon, 0));
}


/**
 *  @detail  Since these are in i,j, with a NEURONS_IN_CORE dimensionality, just
 *	increment the synapse value j, and if it is > NEURONS_IN_CORE, then abort
 *
 */
tw_lpid getSynapseFromSynapse(tw_lpid synapse)
{
	id_type local = LOCAL(synapse);
	_gridIDT j = JSIDE(local);

	j++;
	if (j > NEURONS_IN_CORE) {
		abort();
	}
	return (globalID(CORE(synapse), ISIDE(local), j));
}


tw_lpid getNeuronFromSynapse(tw_lpid synapse)
{
	id_type local = LOCAL(synapse);
	_gridIDT j = JSIDE(local);

	return (globalID(CORE(synapse), AXONS_IN_CORE, j));
}


tw_lpid getSynapseFromAxon(tw_lpid axon)
{
	id_type local = LOCAL(axon);

	return (globalID(CORE(axon), 1, JSIDE(local)));
}


#undef VERIFY_MAPPING
#define VERIFY_MAPPING    1
int n_created = 0;
int s_created = 0;
int a_created = 0;

void nlMap()
{
}


void linearMap()
{
	tw_pe *pe;

	int nlp_per_kp;
	int lpid;
	int kpid;
	int i;
	int j;

	// may end up wasting last KP, but guaranteed each KP has == nLPs
	nlp_per_kp = (int)ceil((double)g_tw_nlp / (double)g_tw_nkp);

	if (!nlp_per_kp) {
		tw_error(TW_LOC, "Not enough KPs defined: %d", g_tw_nkp);
	}

	g_tw_lp_offset = g_tw_mynode * g_tw_nlp;

#if VERIFY_MAPPING
	printf("NODE %ld: nlp %lld, offset %lld\n", g_tw_mynode, g_tw_nlp, g_tw_lp_offset);
#endif

	for (kpid = 0, lpid = 0, pe = NULL; (pe = tw_pe_next(pe)); )
	{
#if VERIFY_MAPPING
		printf("\tPE %lu\n", pe->id);
#endif

		for (i = 0; i < nkp_per_pe; i++, kpid++)
		{
			tw_kp_onpe(kpid, pe);

#if VERIFY_MAPPING
			printf("\t\tKP %d", kpid);
#endif

			for (j = 0; j < nlp_per_kp && lpid < g_tw_nlp; j++, lpid++)
			{
				tw_lpid offLPID = lpid + g_tw_lp_offset;

				//tw_lp_onpe(lpid, pe, localToGlobal(g_tw_lp_offset+lpid));
				tw_lp_onpe(lpid, pe, g_tw_lp_offset + lpid);
				tw_lp_onkp(g_tw_lp[lpid], g_tw_kp[kpid]);

#if VERIFY_MAPPING
				if (0 == j % 20) {
					printf("\n\t\t\t");
				}
				printf("%lld ", lpid + g_tw_lp_offset);
#endif
			}

#if VERIFY_MAPPING
			printf("\n");
#endif
		}
	}

	if (!g_tw_lp[g_tw_nlp - 1]) {
		tw_error(TW_LOC, "Not all LPs defined! (g_tw_nlp=%d)", g_tw_nlp);
	}

	if (g_tw_lp[g_tw_nlp - 1]->gid != g_tw_lp_offset + g_tw_nlp - 1) {
		tw_error(TW_LOC, "LPs not sequentially enumerated!");
	}
}


/**
 *  @detail Since grid mapping works on PCS, here this is grid mapping with neurons.
 *	We have an X,Y,Z setup, where Z is a core. There may be 1 or more cores running
 *	on a PE, but generally there should be < 1 core per PE due to memory limitations.
 */
void tn_cube_mapping()
{
	uint16_t x, y;
	tw_lpid lpid = 0, kpid;
	//tw_lpid numElementsPerKP, vpPerProc;
	//int numSynPerPE, numNerPerPe, numAxPerPE;
	int nlp_per_kp;
	tw_pe *pe;

	/**
	 *  @todo add more dynamic KP assignment
	 */

	nlp_per_kp = (int)ceil((double)g_tw_nlp / (double)g_tw_nkp);

	if (!nlp_per_kp) {
		tw_error(TW_LOC, "Not enough KPs defined: %d", g_tw_nkp);
	}


	g_tw_lp_offset = nodeOffset();

#if VERIFY_MAPPING
	printf("NODE %d: nlp %lld, offset %lld\n", g_tw_mynode, g_tw_nlp, g_tw_lp_offset);
#endif

	for (kpid = 0, lpid = nodeOffset(), pe = NULL; (pe = tw_pe_next(pe)); )
	{
#if VERIFY_MAPPING
		printf("\tPE %lu\n", pe->id);
#endif


		for (int i = 0; i < nkp_per_pe; i++, kpid++)
		{
			tw_kp_onpe(kpid, pe);
#if VERIFY_MAPPING
			printf("\t\tKP %llu", kpid);
#endif

			for (int j = 0; j < nlp_per_kp && lpid < g_tw_nlp; j++)
			{
				tw_lpid lpGlobal = lpid;
				if (!dontSkip(lpid)) {
					lpGlobal = lpid + 1;
				}
				lpGlobal = localToGlobal(lpGlobal);
				tw_lp_onpe(lpid, pe, lpGlobal);
				tw_lp_onkp(g_tw_lp[lpid], g_tw_kp[kpid]);
#if VERIFY_MAPPING
				if (0 == j % 20) {
					printf("\n\t\t\t");
				}
				int lvc = LOCAL(g_tw_lp[lpid]->gid);
				printf("%lld-%i,%i,%i ", lpid + g_tw_lp_offset, CORE(g_tw_lp[lpid]->gid), ISIDE(lvc), JSIDE(lvc));
#endif

				lpid++;
#if VERIFY_MAPPING
				if (0 == j % 20) {
					printf("\n\t\t\t");
				}
				printf("%lld ", lpid + g_tw_lp_offset);
#endif
			}
		}
#if VERIFY_MAPPING
		printf("\n Created \n");
#endif
	}
	if (!g_tw_lp[g_tw_nlp - 1]) {
		tw_error(TW_LOC, "Not all LPs defined! (g_tw_nlp=%d)", g_tw_nlp);
	}

//	for (int i = 0; i < LPS_PER_PE; i ++) {
//		lpid = localToGlobal(i);
//		if(dontSkip(lpid)){
//				//valid LP
//			kpid = i/LP_PER_KP;
//			if(kpid >= g_tw_nkp) {
//				tw_error(TW_LOC, "Attempting to mapping a KPid (%llu) for Global LPid %llu that is beyond g_tw_nkp (%llu)\n",
//						 kpid, lpid, g_tw_nkp );
//			}
//
//			tw_lp_onpe(i, g_tw_pe[0], lpid);
//			if(g_tw_kp[kpid] == NULL){
//				tw_kp_onpe(kpid, g_tw_pe[0]);
//			}
//
//			tw_lp_onkp(tw_getlp(i),g_tw_kp[0]);
//				//tw_lp_settype(i, &model_lps[lpTypeMapper(tw_getlp(i)->gid)]);
//
//
//		}
//
//	}
//		//tw_lp_setup_types();
}


#undef VERIFY_MAPPING
#define VERIFY_MAPPING    1

tw_lpid tn_linear_map(tw_lpid gid)
{
	gid -= lCoreOffset(gid);
	if (gid < AXONS_IN_CORE) {
		return (AXON);
	} else {
		gid -= AXONS_IN_CORE;
		if (gid < SYNAPSES_IN_CORE) {
			return (SYNAPSE);
		}
	}
	return (NEURON);
}


tw_lpid lpTypeMapper(tw_lpid gid)
{
	int neu = 0;
	int syn = 1;
	int ax = 2;
	id_type loc = LOCAL(gid);
	//int b_ax = 0;
	//int e_ax = AXONS_IN_CORE;
	//int b_n = 1;
	//int e_n = NEURONS_IN_CORE;

	_gridIDT i = ISIDE(loc);
		_gridIDT j = JSIDE(loc);

	if (i == 0) {
		a_created++;
		return (ax);
	} else if (i == NEURONS_IN_CORE - 1) {
		n_created++;
		return (neu);
	} else {
		s_created++;

		return (syn);
	}
}


//
//	uint16_t myPEi, myPEj;
//	id_type myPECore;
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



bool dontSkip(tw_lpid gid)
{
	id_type loc = LOCAL(gid);
	id_type jside = JSIDE(loc);
	id_type iside = ISIDE(loc);

	if ((JSIDE(loc) == 0) && (ISIDE(loc) == (NEURONS_IN_CORE))) {
		return (false);
	}
	return (true);
}


void scatterMap()
{
	//int LPsPerPE = SIM_SIZE / tw_nnodes();

	//Create the 1-D table
	tw_lpid *gidArray = NULL;

	gidArray = (tw_lpid *)calloc(sizeof(tw_lpid), SIM_SIZE);
	//Create mappings:
	//use modulus and a single loop for this
	int i;
	for (i = 0; i < LPS_PER_PE; i++)
	{
		tw_lpid id = localToGlobal(i);
		if (dontSkip(id)) {
			gidArray[i] = id;
		} else {
			printf("\nskipped\n");
		}
	}


	printf("\n\n\ntotal lps %i - actual is %i\n PEs per LP is %i  - g_npe: %lu", SIM_SIZE, i, LPS_PER_PE, g_tw_npe);
	//create GID arrays for each LP:

	//myGIDs = (tw_lpid *)malloc(sizeof(tw_lpid)* LPS_PER_PE);
	//wait for alloc to happen.
	MPI_Barrier(MPI_COMM_WORLD);
	//MPI_Scatter(gidArray, LPS_PER_PE, MPI_UINT64_T, myGIDs, LPS_PER_PE, MPI_UINT64_T, 0, MPI_COMM_WORLD);
	//now all GIDs have been seeded to the sim.

	int ct = 0;

	while (ct <= tw_nnodes())
	{
		MPI_Barrier(MPI_COMM_WORLD);
		if ((g_tw_mynode == ct) && (SIM_SIZE < 1024)) {
			printf("LPs/PE %i", LPS_PER_PE);
			printf("PEID #%li HAS \n CORE\ti\tj\tlocal\n", g_tw_mynode);
			int i;

			for (i = 0; i < LPS_PER_PE; i++)
			{
				id_type loc = LOCAL(gidArray[i]);
				tw_peid pe = getPEFromGID(gidArray[i]);

				printf("\t%i\t%i\t%i\t%llu\n", CORE(gidArray[i]), ISIDE(loc), JSIDE(loc), globalToLocalID(gidArray[i]));
				if (pe != g_tw_mynode) {
					printf("ERROR - calced PE %lu is not my PE %li \n", pe, g_tw_mynode);
				}
			}
			printf("for a total of %i elements\n\n", i);
		}

		ct++;
	}


	if (g_tw_mynode == 0) { //free the memory after doing things
		free(gidArray);
	}
}


tw_peid getPEFromGID(tw_lpid gid)
{
	//Given a gid return a PE.
	id_type cc = combVal(gid); ///@todo why does this go to one after dividing?
	id_type res = cc / LPS_PER_PE;

	return ((tw_peid)res);
	//i and j are limited by the number of axons and neurons.
}


tw_lp *globalToLP(tw_lpid gid)
{
	return (g_tw_lp[globalToLocalID(gid)]);
}


tw_lpid globalToLocalID(tw_lpid gid)
{
	id_type cc = combVal(gid);
	tw_lpid peoff = g_tw_mynode * LPS_PER_PE;

	return (cc - peoff);
}


id_type combVal(tw_lpid gid)
{
	id_type i = iVal(gid);
	id_type c = cVal(gid);
	id_type j = jVal(gid);

	return (i + c + j);
}


tw_lpid localToGlobal(tw_lpid local)
{
	local = local + nodeOffset();

	_gridIDT localJ = local % jSizeOffset();
	_gridIDT localI = local / iSizeOffset();
	id_type core = (localJ * jSizeOffset()) / CORE_SIZE;
	//localJ = (localJ * jSizeOffset()) % CORE_SIZE;
	return (globalID(core, localI, localJ));
}
