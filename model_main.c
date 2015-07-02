//
//  model_main.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/17/15.
//
//

#include "model_main.h"

// tw_lptype model_lps[] = {
//    {

//     (init_f)neuron_init, (pre_run_f)pre_run, (event_f)neuron_event,
//     (revent_f)neuron_reverse, (final_f)neuron_final, (map_f)getPEFromGID,
//     sizeof(neuronState)},
//    {

//     (init_f)synapse_init, (pre_run_f)pre_run, (event_f)synapse_event,
//     (revent_f)synapse_reverse, (final_f)synapse_final, (map_f)getPEFromGID,
//     sizeof(synapseState)},

//    {(init_f)axon_init, (pre_run_f)pre_run, (event_f)axon_event,
//     (revent_f)axon_reverse, (final_f)axon_final, (map_f)getPEFromGID,
//     sizeof(axonState)},
//    {0}

//};
tw_lptype model_lps[] = {{

		                         (init_f) neuron_init,  (pre_run_f) pre_run, (event_f) neuron_event,  (revent_f) neuron_reverse,  (final_f) neuron_final,  (map_f) lGidToPE, sizeof(neuronState)},
                         {

		                         (init_f) synapse_init, (pre_run_f) pre_run, (event_f) synapse_event, (revent_f) synapse_reverse, (final_f) synapse_final, (map_f) lGidToPE, sizeof(synapseState)},

                         {       (init_f) axon_init,    (pre_run_f) pre_run, (event_f) axon_event,    (revent_f) axon_reverse,    (final_f) axon_final,    (map_f) lGidToPE, sizeof(axonState)},
                         {       0}

};

double compute_average(double *array, int num_elements) {
	double sum = 0;
	for (int i = 0; i < num_elements; i++) {
		sum += array[i];
	}
	return sum / num_elements;
}

double *create_rand_nums(int num_elements) {
	double *rand_nums = (double *) malloc(sizeof(double) * num_elements);
	// assert(rand_nums != NULL);
	int i;
	for (i = 0; i < num_elements; i++) {
		rand_nums[i] = (rand() / (double) RAND_MAX);
	}
	return rand_nums;
}

int main(int argc, char *argv[]) {
	
	tw_opt_add(app_opt);
	tw_init(&argc, &argv);

		//Some init messages:

	// set up core sizes.
	AXONS_IN_CORE = NEURONS_IN_CORE;
	SYNAPSES_IN_CORE = (NEURONS_IN_CORE * AXONS_IN_CORE);
	CORE_SIZE = SYNAPSES_IN_CORE + NEURONS_IN_CORE + AXONS_IN_CORE;
	SIM_SIZE = CORE_SIZE * CORES_IN_SIM;
	tnMapping = LLINEAR;
	/** g_tw_nlp set here to CORE_SIZE.
	   * @todo check accuracy of this
	   * */
	LPS_PER_PE = SIM_SIZE / tw_nnodes();
	LP_PER_KP = LPS_PER_PE / g_tw_nkp;

	g_tw_events_per_pe = g_tw_nlp * eventAlloc + 2048;
	///@todo enable custom mapping with these smaller LPs.

	if (tnMapping == LLINEAR) {
		g_tw_mapping = LINEAR;
		g_tw_lp_types = model_lps;
		g_tw_lp_typemap = &tn_linear_map;

		// g_tw_custom_initial_mapping = &nlMap;
		// g_tw_custom_lp_global_to_local_map = &globalToLP;
	} else if (tnMapping == CUST_LINEAR) {
		g_tw_mapping = LINEAR;
		g_tw_lp_types = model_lps;
		g_tw_lp_typemap = &lpTypeMapper;
		g_tw_custom_initial_mapping = &nlMap;
		g_tw_custom_lp_global_to_local_map = &globalToLP;
	}

	g_tw_lookahead = LH_VAL;
	// g_tw_clock_rate = CL_VAL;
	// g_tw_nlp = SIM_SIZE - 1;

	g_tw_memory_nqueues = 16;  // give at least 16 memory queue event

	tw_define_lps(LPS_PER_PE, sizeof(Msg_Data), 0);
	tw_lp_setup_types();

	///@todo do we need a custom lookahedad parameter?


	// scatterMap();
	// createLPs();

	// printf("\nCreated %i ax, %i ne, %i se", a_created, n_created, s_created);
	if(g_tw_mynode == 0)
		displayModelSettings();
		//testTiming();


	tw_run();
	// Stats Collection ************************************************************************************88
	_statT totalSOPS = 0;
	_statT totalSynapses = 0;
	MPI_Reduce(&neuronSOPS, &totalSOPS, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Reduce(&synapseEvents, &totalSynapses, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	tw_end();

	if (g_tw_mynode == 0) {  // master node for outputting stats.

		printf("\n ------ TN Benchmark Stats ------- \n");
		printf("Total SOPS(integrate and/or fire): %llu\n", totalSOPS);
		printf("This PE's SOPS: %llu\n", neuronSOPS);
		printf("Total Synapse MSGs sent: %llu\n", totalSynapses);
	}

	return 0;
}

///
/// \details createLPs currently assigns a core's worth of LPs to the PE.
/// @todo need to create better mappingassert(lp->pe->delta_buffer[0] && "increase --buddy_size argument!");.
///
void createLPs() {
	tw_define_lps(CORE_SIZE, sizeof(Msg_Data), 0);
	int neurons = 0;
	int synapses = 0;
	int axons = 0;
	int soff = AXONS_IN_CORE + SYNAPSES_IN_CORE;
	// int noff = CORE_SIZE - NEURONS_IN_CORE;
	for (int i = 0; i < g_tw_nlp; i++) {
		if (i < AXONS_IN_CORE) {
			tw_lp_settype(i, &model_lps[2]);
			axons++;
		} else if (i < soff) {
			tw_lp_settype(i, &model_lps[1]);
			synapses++;
		} else {
			tw_lp_settype(i, &model_lps[0]);
			neurons++;
		}
	}
	// printf("Created %i axons, %i synapses,  %i neurons", axons, synapses,
	// neurons);
}

// neuron gen function helpers

// neuron functions
void neuron_init(neuronState *s, tw_lp *lp) {
	memset(s, 0, sizeof(neuronState));
	if (tnMapping == LLINEAR) {
		s->myCoreID = getCoreFromGID(lp->gid);
		s->myLocalID = lGetNeuNumLocal(lp->gid);
	} else {
		s->myCoreID = getCoreFromGID(lp->gid);

		s->myLocalID = getCoreLocalFromGID(lp->gid);
	}
	//BASIC SOPS SETUP - FOR STRICT BENCHMARK
	if(BASIC_SOP) {
	    s->threshold = s->threshold = tw_rand_integer(lp->rng, THRESHOLD_MIN, THRESHOLD_MAX);
	    for (int i = 0; i < SYNAPSES_IN_CORE; i++) {
		    //See if this neuron is negative:
		    int_fast32_t synMinWeight =  tw_rand_poisson(lp->rng, 1) > 2? - SYNAPSE_WEIGHT_MIN : SYNAPSE_WEIGHT_MIN;

		    s->synapticWeightProb[i] = tw_rand_integer(lp->rng, synMinWeight , SYNAPSE_WEIGHT_MAX);
	    }

	    s->dendriteCore = tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);
	    s->dendriteLocal = tw_rand_integer(lp->rng, 0, AXONS_IN_CORE - 1);

	    if (tnMapping == LLINEAR) {
		    s->dendriteGlobalDest = lGetAxonFromNeu(s->dendriteCore, s->dendriteLocal);
	    } else {
		    s->dendriteGlobalDest = getAxonGlobal(s->dendriteCore, s->dendriteLocal);
	    }

	  }else if (GEN_ON) {  // probabilistic generated mapping
		s->threshold = tw_rand_integer(lp->rng, THRESHOLD_MIN, THRESHOLD_MAX);
		s->negativeThreshold = tw_rand_integer(lp->rng, NEG_THRESHOLD_MIN, NEG_THRESHOLD_MAX);
		s->resetVoltage = tw_rand_integer(lp->rng, RESET_VOLTAGE_MIN, RESET_VOLTAGE_MAX);
		// Randomized selection - calls to various random functions.
		short resetSel = tw_rand_integer(lp->rng, 0, 2);

		///@todo resetSel right now is set to always be 0. Stochastic resets are not very good ATM.
		//resetSel = 0;

		bool stochasticThreshold = tw_rand_poisson(lp->rng, 1) > 3;
		//s->synapticWeightProb =
		//    tw_calloc(TW_LOC, "Neuron", sizeof(_weightT), SYNAPSES_IN_CORE);
		//s->synapticWeightProbSelect =
		//    tw_calloc(TW_LOC, "Neuron", sizeof(bool), SYNAPSES_IN_CORE);
		// select a reset & stochastic reset mode:
		switch (resetSel) {
			case 0:s->doReset = resetNormal;
		        s->reverseReset = reverseResetNormal;
		        break;
			case 1:s->doReset = resetLinear;
		        s->reverseReset = reverseResetLinear;
			default:stochasticThreshold = true;
		        s->doReset = resetNone;
		        s->reverseReset = reverseResetNone;
		        break;
		}
		if (stochasticThreshold == true) {
			// random here as well:
			int sizeInBits = sizeof(s->thresholdPRNMask) * 8;  //  assuming 8 bits per byte;
			///@todo add a variable size to the number of bytes in the range.
			_threshT param = tw_rand_ulong(lp->rng, RAND_RANGE_MIN, RAND_RANGE_MAX);
			s->thresholdPRNMask = (param >= sizeInBits ? -1 : (1 << param) - 1);
			if (s->thresholdPRNMask == -1) { abort(); }
		}

		for (int i = 0; i < SYNAPSES_IN_CORE; i++) {
			/**
			 *  @todo  Enable per neuron probability selection.
			 */
			s->synapticWeightProbSelect[i] = stochasticThreshold;
			//See if this neuron is negative:
			int_fast32_t synMinWeight =  tw_rand_poisson(lp->rng, 1) > 3? - SYNAPSE_WEIGHT_MIN : SYNAPSE_WEIGHT_MIN;

			s->synapticWeightProb[i] = tw_rand_integer(lp->rng, synMinWeight , SYNAPSE_WEIGHT_MAX);
		}

		s->doLeak = linearLeak;
		s->doLeakReverse = revLinearLeak;
		// destinations. again using
		unsigned int calls;
		s->leakRateProb = tw_rand_normal_sd(lp->rng, 0, 10, &calls);
		///@todo Start using stochastic leak functions.
		s->leakWeightProbSelect = false;
		s->leakReversalFlag = tw_rand_integer(lp->rng, 0, 1);

		// randomized output dendrites:
		s->dendriteCore = tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);
		s->dendriteLocal = tw_rand_integer(lp->rng, 0, AXONS_IN_CORE - 1);

		if (tnMapping == LLINEAR) {
			s->dendriteGlobalDest = lGetAxonFromNeu(s->dendriteCore, s->dendriteLocal);
		} else {
			s->dendriteGlobalDest = getAxonGlobal(s->dendriteCore, s->dendriteLocal);
		}
	}
	if (DEBUG_MODE) {
		printf("Neuron %i checking in with GID %llu and dest %llu \n", s->myLocalID, lp->gid, s->dendriteGlobalDest);
	}
}

void setSynapseWeight(neuronState *s, tw_lp *lp, int synapseID) { }

void neuron_event(neuronState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp) {

//	if (BASIC_SOP && (g_tw_synchronization_protocol == OPTIMISTIC || g_tw_synchronization_protocol == OPTIMISTIC_DEBUG)) {
//		tw_snapshot(lp, lp->type->state_sz);
//	}

	if(BASIC_SOP){
	    neuronReceiveMessageBasic(s,tw_now(lp),M,lp);
	  }else {
	neuronReceiveMessage(s, tw_now(lp), M, lp);
	  }

//	if (BASIC_SOP && (g_tw_synchronization_protocol == OPTIMISTIC || g_tw_synchronization_protocol == OPTIMISTIC_DEBUG)) {
//		tw_snapshot_delta(lp, lp->type->state_sz);
//	}

}

void neuron_reverse(neuronState *s, tw_bf *CV, Msg_Data *MCV, tw_lp *lp) {


//	if (BASIC_SOP && (g_tw_synchronization_protocol == OPTIMISTIC || g_tw_synchronization_protocol == OPTIMISTIC_DEBUG)) {
//		tw_snapshot_restore(lp, lp->type->state_sz, lp->pe->cur_event->delta_buddy, lp->pe->cur_event->delta_size);
//		long count = MCV->rndCallCount;
//		while (count--) {
//			tw_rand_reverse_unif(lp->rng);
//		}
//
//	} else {
		neuornReverseState(s, CV, MCV, lp);
		//}


}

void neuron_final(neuronState *s, tw_lp *lp) {
	neuronSOPS += s->SOPSCount;
	printf("neuron %i has %i SOPS \n", lp->gid, s->SOPSCount);
}

// synapse function

void synapse_init(synapseState *s, tw_lp *lp) {
	if (tnMapping == LLINEAR) {
		s->destNeuron = lGetNeuronFromSyn(lp->gid);
		s->destSynapse = 0;
		s->destSynapse = lGetNextSynFromSyn(lp->gid);
		s->mySynapseNum = lGetSynNumLocal(lp->gid);

	}
		/**
		   *  @todo Fix this - there are some logic errors here.
		   */
	else {
		s->destNeuron = getNeuronFromSynapse(lp->gid);
		s->destSynapse = 0;
		int16_t local = LOCAL(lp->gid);
		s->mySynapseNum = ISIDE(local);

		//@todo make this a matrix map - still have linear style of mapping!!!!!
		if (ISIDE(local) == NEURONS_IN_CORE) {
			s->destSynapse = getSynapseFromSynapse(lp->gid);
		}
	}

	s->msgSent = 0;
	if (DEBUG_MODE) {
		printf("Synapse %i checking in with GID %llu and n-dest %llu, s-dest %llu on "
				       "PE %i , CPE %i\n", s->mySynapseNum, lp->gid, s->destNeuron, s->destSynapse, lp->pe->id,
		       lGidToPE(lp->gid));
	}
}

void synapse_event(synapseState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp) {
	long rc = lp->rng->count;
	*(int *) CV = (int) 0;
	// printf("Synapse rcvd msg\n");
	if (s->destSynapse != 0) {
		// generate event to send to next synapse
		s->msgSent++;
		CV->c0 = 1;
		long rc = lp->rng->count;
		tw_event *axe = tw_event_new(s->destSynapse, getNextEventTime(lp), lp);
		Msg_Data *data = (Msg_Data *) tw_event_data(axe);
		data->eventType = SYNAPSE_OUT;
		data->localID = lp->gid;

		tw_event_send(axe);
	}

	// generate event to send to neuron.
	rc = lp->rng->count;
	s->msgSent++;
	CV->c1 = 1;
	rc = lp->rng->count;
	tw_event *axe = tw_event_new(s->destNeuron, getNextEventTime(lp), lp);
	Msg_Data *data = (Msg_Data *) tw_event_data(axe);
	data->eventType = SYNAPSE_OUT;
	data->localID = s->mySynapseNum;
	tw_event_send(axe);
	if (DEBUG_MODE) {
		printf("Synapse received and sent message - sGID %i - sDestSyn %i - sDestNeu %i\n", lp->gid, s->destSynapse,
		       s->destNeuron);
	}
	M->rndCallCount =  lp->rng->count - rc;
}

void synapse_reverse(synapseState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp) {
	// printf("Synape reverse \n");
	long count = M->rndCallCount;
	while (count--) {
		tw_rand_reverse_unif(lp->rng);
	}
	if (CV->c0) { s->msgSent--; }
	if (CV->c1) { s->msgSent--; }
}

void synapse_final(synapseState *s, tw_lp *lp) {
	synapseEvents += s->msgSent;
}

// Axon function.
_idT curAxon = 0;

void axon_init(axonState *s, tw_lp *lp) {
	s->sendMsgCount = 0;
	if (tnMapping == LLINEAR) {
		s->destSynapse = lGetSynFromAxon(lp->gid);
	} else {
		s->destSynapse = getSynapseFromAxon(lp->gid);
		_idT l = LOCAL(lp->gid);

		// tw_printf(TW_LOC, "Axon %i sending message to GID %llu", JSIDE(l),
		// s->destSynapse );
		if (DEBUG_MODE) {
			printf("Axon %i checking in (custom) with gid %llu and dest synapse %llu\n ", JSIDE(l), lp->gid,
			       s->destSynapse);
		}
	}
	tw_stime r = getNextEventTime(lp);
	tw_event *axe = tw_event_new(lp->gid, r, lp);
	Msg_Data *data = (Msg_Data *) tw_event_data(axe);
	data->eventType = AXON_OUT;
	tw_event_send(axe);
	if (DEBUG_MODE) {
		printf("Axon %i checking in with with dest synapse %llu\n", lp->gid, s->destSynapse);
	}
	//printf("message ready at %f",r);
}

void axon_event(axonState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp) {
	// send a message to the attached synapse

	long rc = lp->rng->count;
	tw_event *axe = tw_event_new(s->destSynapse, getNextEventTime(lp), lp);
	Msg_Data *data = (Msg_Data *) tw_event_data(axe);
	data->localID = lp->gid;
	data->eventType = AXON_OUT;

	tw_event_send(axe);
	M->rndCallCount = lp->rng->count - rc;

}

void axon_reverse(axonState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp) {
	if (DEBUG_MODE) {
		printf("axe reverse \n");
	}
	s->sendMsgCount--;

	//tw_snapshot_restore(lp, lp->type->state_sz, lp->pe->cur_event->delta_buddy,lp->pe->cur_event->delta_size);
	long count = M->rndCallCount;
	while (count--) {
		tw_rand_reverse_unif(lp->rng);
	}
}

void axon_final(axonState *s, tw_lp *lp) { }

void pre_run() {

}
void displayModelSettings(){

	for (int i = 0; i < 30; i ++) {
		printf("*");
	}
	printf("\n");
	char * sopMode = BASIC_SOP ? "simplified Neuron Model" : "normal TN Neuron Model";
	printf("* \tNeurons set to %s.\n", sopMode);

		printf("* \tTiming - Big tick occurs at %f\n", getNextBigTick(0));

	for (int i = 0; i < 30; i ++) {
		printf("*");
	}
	printf("\n");
}
