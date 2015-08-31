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
tw_lptype model_lps[] = {
    {
        (init_f)axon_init,
        (pre_run_f)pre_run,
        (event_f)axon_event,
        (revent_f)axon_reverse,
        (final_f)axon_final,
        (map_f)lGidToPE,
        sizeof(axonState) },
    {
        (init_f)synapse_init, (pre_run_f)pre_run,
        (event_f)synapse_event,
        (revent_f)synapse_reverse,
        (final_f)synapse_final,
        (map_f)lGidToPE, sizeof(synapseState)
    },
	{
	(init_f)neuron_init,
	(pre_run_f)pre_run,
	(event_f)neuron_event,
	(revent_f)neuron_reverse,
	(final_f)neuron_final,
	(map_f)lGidToPE, sizeof(neuronState)
	}
	,
			  { 0 } };

int main(int argc, char *argv[])
{

	tw_opt_add(app_opt);
	g_tw_gvt_interval = 512;
	tw_init(&argc, &argv);

//	// set up core sizes.
	AXONS_IN_CORE = NEURONS_IN_CORE;
	SYNAPSES_IN_CORE = (NEURONS_IN_CORE * AXONS_IN_CORE);
	CORE_SIZE = SYNAPSES_IN_CORE + NEURONS_IN_CORE + AXONS_IN_CORE;
	SIM_SIZE = (CORE_SIZE * CORES_IN_SIM);// / tw_nnodes();
	tnMapping = LLINEAR;
	/** g_tw_nlp set here to CORE_SIZE.
	 * @todo check accuracy of this
	 * */
	LPS_PER_PE = SIM_SIZE / tw_nnodes();
	LP_PER_KP = LPS_PER_PE / g_tw_nkp;
//
	g_tw_events_per_pe = SIM_SIZE;//eventAlloc * 9048;//g_tw_nlp * eventAlloc + 4048;
//	///@todo enable custom mapping with these smaller LPs.

	g_tw_lp_typemap = &tn_linear_map;
	g_tw_lp_types = model_lps;


	//if (tnMapping == LLINEAR) {
    //g_tw_mapping = CUSTOM;
    //g_tw_lp_types = model_lps;
		//g_tw_lp_typemap = &tn_linear_map;
   //g_tw_lp_typemap = &clLpTypeMapper;

    //g_tw_custom_initial_mapping = &clMap;
    //g_tw_custom_lp_global_to_local_map = &clLocalFromGlobal;
	// g_tw_1types = model_lps;
	//g_tw_lp_typemap = &lpTypeMapper;
    //g_tw_lp_typemap = &clLpTypeMapper;
    //g_tw_custom_initial_mapping = &clMap;
    //g_tw_custom_lp_global_to_local_map = &clLocalFromGlobal;
//
//	}
//
    g_tw_lookahead = LH_VAL;
//	// g_tw_clock_rate = CL_VAL;
    g_tw_nlp = LPS_PER_PE;
//
//	g_tw_memory_nqueues = 16;  // give at least 16 memory queue event
//
	tw_define_lps(LPS_PER_PE, sizeof(Msg_Data), 0);
    tw_lp_setup_types();
//
//	///@todo do we need a custom lookahedad parameter?
//
//sim_size is
//	// scatterMap();
//	// createLPs();
//
//	// printf("\nCreated %i ax, %i ne, %i se", a_created, n_created, s_created);
	if (g_tw_mynode == 0) {
		displayModelSettings();
	}
//		//testTiming();
//
//
	tw_run();
//	// Stats Collection ************************************************************************************88
	totalSOPS = 0;
	totalSynapses = 0;
    stat_type totalNFire = 0;
	MPI_Reduce(&neuronSOPS, &totalSOPS, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Reduce(&synapseEvents, &totalSynapses, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&fireCount, &totalNFire, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

	statsOut();
	tw_end();
//
	if (g_tw_mynode == 0) {  // master node for outputting stats.
		printf("\n ------ TN Benchmark Stats ------- \n");
		printf("Total SOPS(integrate and/or fire): %zu\n", totalSOPS);
        printf("Total spikes fired by all neurons: %zu\n", totalNFire);
		printf("This PE's SOP: %zu\n", neuronSOPS);
		printf("Total Synapse MSGs sent: %zu\n", totalSynapses);

	}
//
	return (0);
}

void statsOut()
{
	tw_pe *me = g_tw_pe[0];
	tw_statistics s;
	tw_pe *pe;
	tw_kp *kp;
	tw_lp *lp = NULL;


	int i;

	size_t m_alloc, m_waste;

	if (me != g_tw_pe[0]) {
		return;
	}

	if (0 == g_tw_sim_started) {
		return;
	}

	tw_calloc_stats(&m_alloc, &m_waste);
	bzero(&s, sizeof(s));

	for (pe = NULL; (pe = tw_pe_next(pe)); )
	{
		tw_wtime rt;

		tw_wall_sub(&rt, &pe->end_time, &pe->start_time);

		s.s_max_run_time = ROSS_MAX(s.s_max_run_time, tw_wall_to_double(&rt));
		s.s_nevent_abort += pe->stats.s_nevent_abort;
		s.s_pq_qsize += tw_pq_get_size(me->pq);

		s.s_nsend_net_remote += pe->stats.s_nsend_net_remote;
		s.s_nsend_loc_remote += pe->stats.s_nsend_loc_remote;

		s.s_nsend_network += pe->stats.s_nsend_network;
		s.s_nread_network += pe->stats.s_nread_network;
		s.s_nsend_remote_rb += pe->stats.s_nsend_remote_rb;

		s.s_total += pe->stats.s_total;
		s.s_net_read += pe->stats.s_net_read;
		s.s_gvt += pe->stats.s_gvt;
		s.s_fossil_collect += pe->stats.s_fossil_collect;
		s.s_event_abort += pe->stats.s_event_abort;
		s.s_event_process += pe->stats.s_event_process;
		s.s_pq += pe->stats.s_pq;
		s.s_rollback += pe->stats.s_rollback;
		s.s_cancel_q += pe->stats.s_cancel_q;
		s.s_pe_event_ties += pe->stats.s_pe_event_ties;
		s.s_min_detected_offset = g_tw_min_detected_offset;
		s.s_avl += pe->stats.s_avl;
		s.s_buddy += pe->stats.s_buddy;
		s.s_lz4 += pe->stats.s_lz4;
		s.s_events_past_end += pe->stats.s_events_past_end;

		for (i = 0; i < g_tw_nkp; i++)
		{
			kp = tw_getkp(i);
			s.s_nevent_processed += kp->s_nevent_processed;
			s.s_e_rbs += kp->s_e_rbs;
			s.s_rb_total += kp->s_rb_total;
			s.s_rb_secondary += kp->s_rb_secondary;
		}

		for (i = 0; i < g_tw_nlp; i++)
		{
			lp = tw_getlp(i);
			if (lp->type->final) {
				(*lp->type->final)(lp->cur_state, lp);
			}
		}
	}

	s.s_fc_attempts = g_tw_fossil_attempts;
	s.s_net_events = s.s_nevent_processed - s.s_e_rbs;
	s.s_rb_primary = s.s_rb_total - s.s_rb_secondary;

	s = *(tw_net_statistics(me, &s));

	if (!tw_ismaster()) {
		return;
	}



	//
	//printf("\n\n %i", s.s_pe_event_ties);
	//tabular data:
	//NP  - CORES - Neurons per core - Net Events - Rollbacks - Running Time	- SOP
	printf("\n\n");
	printf("Nodes,CORES,Neurons/Core,Net Events,Rollbacks,Run Time,Total SOP,Threshold Min,Threshold Max"
	    ",NegativeThresholdMin,NegativeThresholdMax,Synapse Weight Min,Synapse Weight Max,EvtTies\n");
	printf("%u,%i,%i,%llu,%zu,%f,%zu,", tw_nnodes(), CORES_IN_SIM, NEURONS_IN_CORE, s.s_net_events, s.s_rollback, s.s_max_run_time, totalSOPS);
	printf("%zu,"
	    "%zu,"
	    "%zu,"
	    "%zu,"
	    "%d,"
	    "%d,"
	    "%llu\n", THRESHOLD_MIN, THRESHOLD_MAX, NEG_THRESHOLD_MIN, NEG_THRESHOLD_MAX, SYNAPSE_WEIGHT_MIN, SYNAPSE_WEIGHT_MAX, s.s_pe_event_ties);
	if (BULK_MODE) {
		fprintf(stderr, "%u,%i,%i,%llu,%zu,%f,%zu,%zu,%zu,%zu,%zu,%u,%u,", tw_nnodes(), CORES_IN_SIM,
		    NEURONS_IN_CORE, s.s_net_events, s.s_rollback, s.s_max_run_time, totalSOPS, THRESHOLD_MIN, THRESHOLD_MAX,
		    NEG_THRESHOLD_MIN, NEG_THRESHOLD_MAX, SYNAPSE_WEIGHT_MIN, SYNAPSE_WEIGHT_MAX);
		fprintf(stderr, "%llu", s.s_pe_event_ties);
	}
}




///
/// \details createLPs currently assigns a core's worth of LPs to the PE.
/// @todo need to create better mappingassert(lp->pe->delta_buffer[0] && "increase --buddy_size argument!");.
///
void createLPs()
{
	tw_define_lps(CORE_SIZE, sizeof(Msg_Data), 0);
	int neurons = 0;
	int synapses = 0;
	int axons = 0;
	int soff = AXONS_IN_CORE + SYNAPSES_IN_CORE;
	// int noff = CORE_SIZE - NEURONS_IN_CORE;
	for (tw_lpid i = 0; i < g_tw_nlp; i++)
	{
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
void createSimpleNeuron(neuronState *s, tw_lp *lp){
    //Rewrote this function to have a series of variables that are easier to read.
    //Since init time is not so important, readability wins here.


    //AutoGenerated test neurons:
    bool synapticConnectivity[NEURONS_IN_CORE];
    short G_i[NEURONS_IN_CORE];
    int sigma[4];
    int S[4];
    bool b[4];

    bool epsilon = 0;
    bool sigma_l = 0;
    short lambda = 1;
    bool c = false;
    short TM = 0;

    short VR = 0;

    short sigmaVR = 1;
    short gamma = 0;

    bool kappa = false;

    int signalDelay = 0;



    //per synapse weight / connectivity gen:
    for(int i = 0; i < NEURONS_IN_CORE; i ++) {
        //s->synapticConnectivity[i] = tw_rand_integer(lp->rng, 0, 1);
        //s->axonTypesp[i] = 1; ///! Set axon types to one, since we are just testing performance.
        G_i[i] = tw_rand_integer(lp->rng, 0, 3);
		synapticConnectivity[i] = 0;

			//synapticConnectivity[i] = tw_rand_integer(lp->rng, 0, 1);

    }
	synapticConnectivity[lGetNeuNumLocal(lp->gid)] = 1;

    for(int i = 0; i < 4; i ++){
        //int ri = tw_rand_integer(lp->rng, -1, 0);
        unsigned int mk = tw_rand_integer(lp->rng, 0, 1);

        //sigma[i] = (!ri * 1) + (-1 & ri))
        sigma[i] = (mk ^ (mk - 1)) * 1;

        S[i] = 1;
        b[i] = 0;
    }

    weight_type alpha = tw_rand_integer(lp->rng, THRESHOLD_MIN, THRESHOLD_MAX);
    weight_type beta = tw_rand_integer(lp->rng, (NEG_THRESH_SIGN * NEG_THRESHOLD_MIN), NEG_THRESHOLD_MAX);


//    GlobalID g;
//    g.raw = lp->gid;
//    id_type core = g.core;
//    id_type nid = g.local;





    initNeuron(lGetCoreFromGID(lp->gid), lGetNeuNumLocal(lp->gid), synapticConnectivity, G_i, sigma, S, b, epsilon, sigma_l, lambda, c, alpha, beta, TM, VR, sigmaVR, gamma, kappa, s, signalDelay,0,0); //we re-define the destination axons here, rather than use the constructor.


    /**@note This random setup will create neurons that have an even chance of getting an axon inside thier own core
     * vs an external core. The paper actually capped this value at something like 20%. @todo - make this match the
     * paper if performance is slow. * */
     s->dendriteCore = tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);
     s->dendriteLocal = tw_rand_integer(lp->rng, 0, AXONS_IN_CORE - 1);

//     if (tnMapping == LLINEAR) {
         s->dendriteGlobalDest = lGetAxonFromNeu(s->dendriteCore, s->dendriteLocal);
//     }
//     else {
//     s->dendriteGlobalDest = getAxonGlobal(s->dendriteCore, s->dendriteLocal);
//     }
     if (DEBUG_MODE) {
     printf("Neuron %llu checking in with GID %llu and dest %llu \n", s->myLocalID, lp->gid, s->dendriteGlobalDest);
     }


}
// neuron functions
void neuron_init(neuronState *s, tw_lp *lp) {

    createSimpleNeuron(s, lp);




}


void setSynapseWeight(neuronState *s, tw_lp *lp, int synapseID)
{
}


void neuron_event(neuronState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp)
{
	long start_count = lp->rng->count;
	//if delta is on or basic mode is on, take a snapshot for delta encoding
	if (TW_DELTA &&
	    (g_tw_synchronization_protocol == OPTIMISTIC || g_tw_synchronization_protocol == OPTIMISTIC_DEBUG)) {
		tw_snapshot(lp, lp->type->state_sz);
			//printf("Neuron snapshot saved");
	}

    neuronReceiveMessage(s, M, lp);


	//basic mode removes leak and stochastic reverse threshold functions.
//	if (BASIC_SOP) {
//		neuronReceiveMessageBasic(s, tw_now(lp), M, lp);
//	}else {
//		neuronReceiveMessage(s, tw_now(lp), M, lp);
	//}

	//again, only take the delta in basic neuron mode or in delta mode.
	if (TW_DELTA &&
			(g_tw_synchronization_protocol == OPTIMISTIC || g_tw_synchronization_protocol == OPTIMISTIC_DEBUG)) {
		tw_snapshot_delta(lp, lp->type->state_sz);
	}
	M->rndCallCount = lp->rng->count - start_count;
}


void neuron_reverse(neuronState *s, tw_bf *CV, Msg_Data *MCV, tw_lp *lp)
{
	long count = MCV->rndCallCount;

	if (TW_DELTA) {
			tw_snapshot_restore(lp, lp->type->state_sz, lp->pe->cur_event->delta_buddy, lp->pe->cur_event->delta_size);

	}
	else { //ReverseState is needed when not using delta encoding. Since basic mode implies delta, this only runs when delta is off and neurons are in normal sim mode.
		neuronReverseState(s,CV,MCV,lp);
        //neuronReverseState()
	}

	//Roll Back random calls
		while (count--)
	{
		tw_rand_reverse_unif(lp->rng);
	}

}


void neuron_final(neuronState *s, tw_lp *lp)
{
	neuronSOPS += s->SOPSCount;
	//printf("neuron %i has %i SOPS \n", lp->gid, s->SOPSCount);
    fireCount += s->fireCount;
}


// synapse function

void synapse_init(synapseState *s, tw_lp *lp)
{
	s->destNeuron = lGetNeuronFromSyn(lp->gid);
	s->destSynapse = lGetNextSynFromSyn(lp->gid);
	s->mySynapseNum = lGetSynNumLocal(lp->gid);
	//if (tnMapping == LLINEAR) {
		//s->destNeuron = clGetNeuronFromSynapse(lp->gid);
		//s->destSynapse = 0;
		//s->destSynapse = clGetSynapseFromSynapse(lp->gid);
//		s->mySynapseNum = clGetSynapseFromSynapse\(lp->gid);

    //GlobalID g;
    //g.raw = lp->gid;
    //s->mySynapseNum = g.local;

	//}

	/**
	 *  @todo Fix this - there are some logic errors here.
	 */
//	else {
//		s->destNeuron = getNeuronFromSynapse(lp->gid);
//		s->destSynapse = 0;
//		int16_t local = LOCAL(lp->gid);
//		s->mySynapseNum = ISIDE(local);
//
//		//@todo make this a matrix map - still have linear style of mapping!!!!!
//		if (ISIDE(local) == NEURONS_IN_CORE) {
//			s->destSynapse = getSynapseFromSynapse(lp->gid);
//		}
//	}

	s->msgSent = 0;
//	if (DEBUG_MODE) {
//		printf("Synapse %i checking in with GID %llu and n-dest %llu, s-dest %llu on "
//		    "PE %lu , CPE %lu\n", s->mySynapseNum, lp->gid, s->destNeuron, s->destSynapse, lp->pe->id,
//		    lGidToPE(lp->gid));
//	}
}



void synapse_event(synapseState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp)
{
	long rc = lp->rng->count;
    //s->destNeuron = clGetNeuronFromSynapse(lp->gid);
    //s->destSynapse = clGetSynapseFromSynapse(lp->gid);
	//s->destNeuron = lGetNeuronFromSyn(lp->gid);
	//s->destSynapse = lGetNextSynFromSyn(lp->gid);
	//*(int *)CV = (int)0;
	// printf("Synapse rcvd msg\n");
	if (s->destSynapse != 0) {
		// generate event to send to next synapse
		s->msgSent++;
		//CV->c0 = 1;
		tw_event *axe = tw_event_new(s->destSynapse, getNextEventTime(lp), lp);
		Msg_Data *data = (Msg_Data *)tw_event_data(axe);
		data->eventType = SYNAPSE_OUT;
		data->localID = lp->gid;
		data->axonID = M->axonID;

		tw_event_send(axe);
	}
	// generate event to send to neuron.
	s->msgSent++;
	//CV->c1 = 1;

	tw_event *axe = tw_event_new(s->destNeuron, getNextEventTime(lp), lp);
	Msg_Data *data = (Msg_Data *)tw_event_data(axe);
	data->eventType = SYNAPSE_OUT;
	data->localID = s->mySynapseNum;
	data->axonID = M->axonID;
	tw_event_send(axe);
	if (DEBUG_MODE) {
		printf("Synapse received and sent message - sGID %llu - sDestSyn %llu - sDestNeu %llu\n", lp->gid, s->destSynapse,
		    s->destNeuron);
	}
	M->rndCallCount = lp->rng->count - rc;
}


void synapse_reverse(synapseState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp)
{
	if(s->destSynapse != 0){
		s->msgSent --;
	}
	s->msgSent --;

	long count = M->rndCallCount;
	while (count--)
	{
		tw_rand_reverse_unif(lp->rng);
	}

}


void synapse_final(synapseState *s, tw_lp *lp)
{
	synapseEvents += s->msgSent;
}


// Axon function.
id_type curAxon = 0;

void axon_init(axonState *s, tw_lp *lp)
{
	s->sendMsgCount = 0;
	s->axonID = lGetAxeNumLocal(lp->gid);
	s->destSynapse = lGetSynFromAxon(lp->gid);


	//if (tnMapping == LLINEAR) {
	//	s->destSynapse = clgetSynapseFromAxon(lp->gid);
    //GlobalID g;
    //g.raw = lp->gid;
    //s->axonID = g.local;

    //		s->axonID = lGetAxeNumLocal(lp->gid);
//	} else {
//		s->destSynapse = getSynapseFromAxon(lp->gid);
//		id_type l = LOCAL(lp->gid);
//
//		// tw_printf(TW_LOC, "Axon %i sending message to GID %llu", JSIDE(l),
//		// s->destSynapse );
//		if (DEBUG_MODE) {
//			printf("Axon %i checking in (custom) with gid %llu and dest synapse %llu\n ", JSIDE(l), lp->gid,
//			    s->destSynapse);
//		}
//	}
    //printf("Axon GIDVAL:(%i,%i), INTVAL(%i),  init message sending to gid %i ...\n",g.local, g.core, s->axonID,lp->gid);
	tw_stime r = getNextEventTime(lp);
	tw_event *axe = tw_event_new(lp->gid, r, lp);
	Msg_Data *data = (Msg_Data *)tw_event_data(axe);
	data->eventType = AXON_OUT;
	data->axonID = s->axonID;
	tw_event_send(axe);
	if (DEBUG_MODE) {

		printf("Axon %zu checking in with dest synapse %zu\n", lp->gid, s->destSynapse);
	}
	//printf("message ready at %f",r);
}


void axon_event(axonState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp)
{
	// send a message to the attached synapse

	long rc = lp->rng->count;

	tw_event *axe = tw_event_new(s->destSynapse, getNextEventTime(lp), lp);
	Msg_Data *data = (Msg_Data *)tw_event_data(axe);

	data->localID = lp->gid;
	data->eventType = AXON_OUT;
	data->axonID = s->axonID;
	//printf("Axon sending message to synapse %zu\n", s->destSynapse);
	tw_event_send(axe);
	s->sendMsgCount ++;
	M->rndCallCount = lp->rng->count - rc;
}


void axon_reverse(axonState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp)
{
	if (DEBUG_MODE) {
		printf("axe reverse \n");
	}
	s->sendMsgCount--;

	//tw_snapshot_restore(lp, lp->type->state_sz, lp->pe->cur_event->delta_buddy,lp->pe->cur_event->delta_size);
	long count = M->rndCallCount;
	while (count--)
	{
		tw_rand_reverse_unif(lp->rng);
	}
}


void axon_final(axonState *s, tw_lp *lp)
{
}


void pre_run()
{
}


void displayModelSettings()
{
	for (int i = 0; i < 30; i++)
	{
		printf("*");
	}
	double cores_per_node = CORES_IN_SIM / tw_nnodes() ;
	printf("\n");
	char *sopMode = BASIC_SOP ? "simplified Neuron Model" : "normal TN Neuron Model";
	printf("* \tNeurons set to %s.\n", sopMode);
	printf("* \t %i Neurons per core, %i cores in sim.\n", NEURONS_IN_CORE, CORES_IN_SIM);
	printf("* \t %zd cores per PE, giving %zu LPs per pe.\n", cores_per_node, g_tw_nlp);
	printf("* \t Neuron stats:\n");
	printf("* \tCalculated sim_size is %u\n", SIM_SIZE);

	//printf("%-10s", "title");


	//printf("* \tTiming - Big tick occurs at %f\n", getNextBigTick(0));

	for (int i = 0; i < 30; i++)
	{
		printf("*");
	}
	printf("\n");
}

/* Old Neuron Init Function
 void neuron_init(neuronState *s, tw_lp *lp)
 {
	//memset(s, 0, sizeof(neuronState));
	if (tnMapping == LLINEAR) {
 s->myCoreID = getCoreFromGID(lp->gid);
 s->myLocalID = lGetNeuNumLocal(lp->gid);
	} else {
 s->myCoreID = getCoreFromGID(lp->gid);

 s->myLocalID = getCoreLocalFromGID(lp->gid);
	}
	//BASIC SOPS SETUP - FOR STRICT BENCHMARK
	if (BASIC_SOP) {

 s->threshold = s->threshold = tw_rand_integer(lp->rng, THRESHOLD_MIN, THRESHOLD_MAX);
 for (int i = 0; i < SYNAPSES_IN_CORE; i++)
 {
 //See if this neuron is negative:
 int_fast32_t synMinWeight = tw_rand_poisson(lp->rng, 1) > 2 ? -SYNAPSE_WEIGHT_MIN : SYNAPSE_WEIGHT_MIN;


 for (int i = 0; i < AXONS_IN_CORE; i++)
 {
 s->weightSelect[i] = tw_rand_integer(lp->rng, 0, 3);
 }
 //set up type weights:
 for (int i = 0; i < 4; i++)
 {
 s->axonWeightProb[i] = tw_rand_integer(lp->rng, -SYNAPSE_WEIGHT_MIN, SYNAPSE_WEIGHT_MAX);
 s->axonProbSelect[i] = tw_rand_poisson(lp->rng, 1) > RAND_WT_PROB;
 }
 }

 s->dendriteCore = tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);
 s->dendriteLocal = tw_rand_integer(lp->rng, 0, AXONS_IN_CORE - 1);

 if (tnMapping == LLINEAR) {
 s->dendriteGlobalDest = lGetAxonFromNeu(s->dendriteCore, s->dendriteLocal);
 } else {
 s->dendriteGlobalDest = getAxonGlobal(s->dendriteCore, s->dendriteLocal);
 }
	}else if (GEN_ON) {    // probabilistic generated mapping
 s->threshold = tw_rand_integer(lp->rng, THRESHOLD_MIN, THRESHOLD_MAX);
 s->negativeThreshold = tw_rand_integer(lp->rng, NEG_THRESHOLD_MIN, NEG_THRESHOLD_MAX);
 s->resetVoltage = tw_rand_integer(lp->rng, RESET_VOLTAGE_MIN, RESET_VOLTAGE_MAX);
 // Randomized selection - calls to various random functions.
 short resetSel = tw_rand_integer(lp->rng, 0, 2);
 bool stochasticThreshold = tw_rand_poisson(lp->rng, 1) > 3;
 //s->synapticWeightProb =
 //    tw_calloc(TW_LOC, "Neuron", sizeof(_weightT), SYNAPSES_IN_CORE);
 //s->synapticWeightProbSelect =
 //    tw_calloc(TW_LOC, "Neuron", sizeof(bool), SYNAPSES_IN_CORE);
 // select a reset & stochastic reset mode:
 switch (resetSel)
 {
 case 0:
 s->doReset = resetNormal;
 s->reverseReset = reverseResetNormal;
 break;

 case 1:
 s->doReset = resetLinear;
 s->reverseReset = reverseResetLinear;

 default:
 stochasticThreshold = true;
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
 if (s->thresholdPRNMask == -1) {
 abort();
 }
 }


 for (int i = 0; i < AXONS_IN_CORE; i++)
 {
 s->weightSelect[i] = tw_rand_integer(lp->rng, 0, 3);
 }
 //set up type weights:
 for (int i = 0; i < 4; i++)
 {
 s->axonWeightProb[i] = tw_rand_integer(lp->rng, -SYNAPSE_WEIGHT_MIN, SYNAPSE_WEIGHT_MAX);
 s->axonProbSelect[i] = RAND_WT_PROB < tw_rand_poisson(lp->rng, 1);
 }

 //old looop
 //		for (int i = 0; i < SYNAPSES_IN_CORE; i++) {
 //			/**
 //			 *  @todo  Enable per neuron probability selection.
 //
//
//			s->synapticWeightProbSelect[i] = stochasticThreshold;
//			//See if this neuron is negative:
//			int_fast32_t synMinWeight =  tw_rand_poisson(lp->rng, 1) > 3? - SYNAPSE_WEIGHT_MIN : SYNAPSE_WEIGHT_MIN;
//
//			s->synapticWeightProb[i] = tw_rand_integer(lp->rng, synMinWeight , SYNAPSE_WEIGHT_MAX);
//		}
}

s->doLeak = linearLeak;
s->doLeakReverse = revLinearLeak;

// destinations. again using
unsigned int calls;
s->leakRateProb = tw_rand_normal_sd(lp->rng, 0, 10, &calls);
///@todo Start using stochastic leak functions.
s->leakWeightProbSelect = RAND_WT_PROB < tw_rand_poisson(lp->rng, 1);
s->leakReversalFlag = tw_rand_integer(lp->rng, 0, 1);

// randomized output dendrites:

/** @note This random setup will create neurons that have an even chance of getting an axon inside thier own core
 * vs an external core. The paper actually capped this value at something like 20%. @todo - make this match the
 * paper if performance is slow. *
s->dendriteCore = tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);
s->dendriteLocal = tw_rand_integer(lp->rng, 0, AXONS_IN_CORE - 1);

if (tnMapping == LLINEAR) {
    s->dendriteGlobalDest = lGetAxonFromNeu(s->dendriteCore, s->dendriteLocal);
} else {
    s->dendriteGlobalDest = getAxonGlobal(s->dendriteCore, s->dendriteLocal);
}
if (DEBUG_MODE) {
    printf("Neuron %i checking in with GID %llu and dest %llu \n", s->myLocalID, lp->gid, s->dendriteGlobalDest);
}
}
*/
