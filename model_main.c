//
//  model_main.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/17/15.
//
//

#include "model_main.h"

 tw_lptype model_lps[] = {{

                             (init_f)neuron_init,
                             (pre_run_f)pre_run,
                             (event_f)neuron_event,
                             (revent_f)neuron_reverse,
                             (final_f)neuron_final,
                             (map_f)getPEFromGID,
                             sizeof(neuronState)},
                         {

                             (init_f)synapse_init,
                             (pre_run_f)pre_run,
                             (event_f)synapse_event,
                             (revent_f)synapse_reverse,
                             (final_f)synapse_final,
                             (map_f)getPEFromGID,
                             sizeof(synapseState)},

                         {(init_f)axon_init,
                          (pre_run_f)pre_run,
                          (event_f)axon_event,
                          (revent_f)axon_reverse,
                          (final_f)axon_final,
                          (map_f)getPEFromGID,
                          sizeof(axonState)},
                         {0}

};
double compute_average(double *array, int num_elements) {
	double sum = 0;
	for(int i = 0; i < num_elements; i ++){
		sum+= array[i];

	}
	return sum / num_elements;
}
double *create_rand_nums(int num_elements) {
	double *rand_nums = (double *)malloc(sizeof(double) * num_elements);
		//assert(rand_nums != NULL);
	int i;
	for (i = 0; i < num_elements; i++) {
		rand_nums[i] = (rand() / (double)RAND_MAX);
	}
	return rand_nums;
}
int main(int argc, char *argv[]) {
  //set up core sizes.
	SYNAPSES_IN_CORE = (NEURONS_IN_CORE * AXONS_IN_CORE);
	CORE_SIZE = SYNAPSES_IN_CORE + NEURONS_IN_CORE  + AXONS_IN_CORE;
	SIM_SIZE = CORE_SIZE * CORES_IN_SIM;
	tnMapping = LLINEAR;


	tw_opt_add(app_opt);

	tw_init(&argc, &argv);
	/** g_tw_nlp set here to CORE_SIZE.
	 * @todo check accuracy of this
	 * */
	LPS_PER_PE = SIM_SIZE / tw_nnodes();
	LP_PER_KP = LPS_PER_PE / g_tw_nkp;

	g_tw_events_per_pe = CORE_SIZE * eventAlloc;
  ///@todo enable custom mapping with these smaller LPs.


	if(tnMapping == LLINEAR){
	g_tw_mapping = LINEAR;
	g_tw_lp_types = model_lps;
	g_tw_lp_typemap = &tn_linear_map;
			//g_tw_custom_initial_mapping = &nlMap;
			//g_tw_custom_lp_global_to_local_map = &globalToLP;
	}
	else if (tnMapping == CUST_LINEAR){
		g_tw_mapping = LINEAR;
		g_tw_lp_types = model_lps;
		g_tw_lp_typemap = &lpTypeMapper;
		g_tw_custom_initial_mapping = &nlMap;
		g_tw_custom_lp_global_to_local_map = &globalToLP;
	}


	g_tw_nlp = SIM_SIZE - 1;

	tw_define_lps(LPS_PER_PE, sizeof(Msg_Data), 0);
	tw_lp_setup_types();

  ///@todo do we need a custom lookahedad parameter?

		//MPI TESTING

		//scatterMap();
		//createLPs();



	printf("\nCreated %i ax, %i ne, %i se", a_created, n_created, s_created);
	tw_run();
	tw_end();

	return 0;
}


///
/// \details createLPs currently assigns a core's worth of LPs to the PE.
/// @todo need to create better mapping.
///
void createLPs() {
   tw_define_lps(CORE_SIZE, sizeof(Msg_Data), 0);
	int neurons = 0;
	int synapses = 0;
	int axons = 0;
	int soff = AXONS_IN_CORE + SYNAPSES_IN_CORE;
		//int noff = CORE_SIZE - NEURONS_IN_CORE;
  for (int i = 0; i < g_tw_nlp; i++) {
	  if(i < AXONS_IN_CORE){
			  tw_lp_settype(i, &model_lps[2]);
		  axons++;
	  }
	  else if (i < soff) {
			  tw_lp_settype(i, &model_lps[1]);
		  synapses++;
	  }
	  else {
			  tw_lp_settype(i, &model_lps[0]);
		  neurons ++;

	  }
  }
		//printf("Created %i axons, %i synapses,  %i neurons", axons, synapses, neurons);
}



//neuron gen function helpers


//neuron functions
void neuron_init(neuronState *s, tw_lp *lp)
{
  memset(s,0,sizeof(neuronState));
	if(tnMapping == LLINEAR){
		s->myCoreID = getCoreFromGID(lp->gid);
		s->myLocalID = lgetNeuNumLocal(lp->gid);
	}
	else{
	s->myCoreID = getCoreFromGID(lp->gid);

	s->myLocalID = getCoreLocalFromGID(lp->gid);
	}

  if(GEN_ON){ //probabilistic generated mapping
       s->threshold = tw_rand_integer(lp->rng,THRESHOLD_MIN, THRESHOLD_MAX);
       s->negativeThreshold = tw_rand_integer(lp->rng, NEG_THRESHOLD_MIN, NEG_THRESHOLD_MAX );
       s->resetVoltage = tw_rand_integer(lp->rng, RESET_VOLTAGE_MIN, RESET_VOLTAGE_MAX);
       //Randomized selection - calls to various random functions.
       long resetSel = tw_rand_integer(lp->rng, 0,2);
       bool stochasticThreshold = tw_rand_poisson(lp->rng,1) > 3;
       s->synapticWeightProb = tw_calloc(TW_LOC, "Neuron", sizeof(_weightT), SYNAPSES_IN_CORE);
       s->synapticWeightProbSelect = tw_calloc(TW_LOC, "Neuron", sizeof(bool), SYNAPSES_IN_CORE);
       //select a reset & stochastic reset mode:
       switch (resetSel) {
         case 0:
           s->doReset =  resetNormal;
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
       if(stochasticThreshold == true) {
           //random here as well:
           int sizeInBits = sizeof(s->thresholdPRNMask) * 8; //  assuming 8 bits per byte;
           ///@todo add a variable size to the number of bytes in the range.
           _threshT param = tw_rand_ulong(lp->rng, RAND_RANGE_MIN, RAND_RANGE_MAX);
           s->thresholdPRNMask = (param >= sizeInBits ? -1 : (1 <<  param) - 1);
           if(s->thresholdPRNMask == -1)
             abort();

         }

       for(int i = 0; i < SYNAPSES_IN_CORE; i ++) {
           s->synapticWeightProbSelect[i] = stochasticThreshold;
           s->synapticWeightProb[i] = tw_rand_integer(lp->rng, SYNAPSE_WEIGHT_MIN, SYNAPSE_WEIGHT_MAX);

         }
       ///@todo add more leak functions
       s->doLeak = linearLeak;
       s->doLeakReverse = revLinearLeak;
       //destinations. again using
       unsigned int calls;
       s->leakRateProb = tw_rand_normal_sd(lp->rng,0,10,&calls);
       s->leakWeightProbSelect =  false;
       s->leakReversalFlag = tw_rand_integer(lp->rng, 0,1);

       //randomized output dendrites:
       s->dendriteCore = tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);
       s->dendriteLocal = tw_rand_integer(lp->rng, 0, AXONS_IN_CORE - 1);

	  if(tnMapping == LLINEAR) {
		  s->dendriteGlobalDest = lGetAxonFromNeu(s->dendriteCore, s->dendriteLocal);
	  }
	  else {
       s->dendriteGlobalDest = getAxonGlobal(s->dendriteCore, s->dendriteLocal);
	  }

    }
	printf("Neuron %i checking in with GID %llu and dest %llu \n",s->myLocalID, lp->gid, s->dendriteGlobalDest);

}

void setSynapseWeight(neuronState *s, tw_lp *lp, int synapseID)
{

}


void neuron_event(neuronState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp)
{
		//printf("Neuron recvd msg\n");
  neuronReceiveMessage(s,tw_now(lp),M,lp);

}



void neuron_reverse(neuronState *s, tw_bf *CV, Msg_Data *MCV, tw_lp *lp)
{
		printf("neuron reverse \n");
  neuornReverseState(s,CV, MCV, lp);
}


void neuron_final(neuronState *s, tw_lp *lp)
{

}

//synapse function

void synapse_init(synapseState *s, tw_lp *lp)
{
	if(tnMapping == LLINEAR){
		s->destNeuron = lGetNeuronFromSyn(lp->gid);
		s->destSynapse =0;
		s->destSynapse = lGetNextSynFromSyn(lp->gid);
		s->mySynapseNum = lGetSynNumLocal(lp->gid);

	}
	/**
	 *  @todo Fix this - there are some logic errors here.
	 */
	else{
		s->destNeuron = getNeuronFromSynapse(lp->gid);
		s->destSynapse = 0;
	int16_t local = LOCAL(lp->gid);
		s->mySynapseNum = ISIDE(local);

  //@todo make this a matrix map - still have linear style of mapping!!!!!
  if(ISIDE(local) == NEURONS_IN_CORE) {
    s->destSynapse = getSynapseFromSynapse(lp->gid);
    }
	}

  s->msgSent = 0;
	printf("Synapse %i,%i checking in with GID %llu and n-dest %llu, s-dest %llu \n",s->mySynapseNum, s->mySynapseNum,lp->gid, s->destNeuron,s->destSynapse);



}


void synapse_event(synapseState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp)
{
  long rc = lp->rng->count;
		//printf("Synapse rcvd msg\n");
  if(s->destNeuron != 0){
      //generate event to send to next synapse
      s->msgSent ++;
      CV->c0  = 1;
      long rc = lp->rng->count;
      tw_event *axe = tw_event_new(s->destSynapse,getNextEventTime(lp),lp);
      Msg_Data *data = (Msg_Data *) tw_event_data(axe);
      data->eventType = SYNAPSE_OUT;

      tw_event_send(axe);
      M->rndCallCount = lp->rng->count - rc;

    }

  //generate event to send to neuron.
  rc = lp->rng->count;
  s->msgSent ++;
  CV->c1 = 1;
   rc = lp->rng->count;
  tw_event *axe = tw_event_new(s->destNeuron,getNextEventTime(lp),lp);
  Msg_Data *data = (Msg_Data *) tw_event_data(axe);
  data->eventType = SYNAPSE_OUT;

  tw_event_send(axe);
  M->rndCallCount = lp->rng->count - rc;

}


void synapse_reverse(synapseState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp)
{
	printf("Synape reverse \n");
  long count = M->rndCallCount;
  while (count--) {
      tw_rand_reverse_unif(lp->rng);
  }
  if(CV->c0)
    s->msgSent --;
  if(CV->c1)
    s->msgSent --;
}


void synapse_final(synapseState *s, tw_lp *lp)
{

}

// Axon function.
_idT curAxon =0;
void axon_init(axonState *s, tw_lp *lp)
{
  s->sendMsgCount = 0;
	if(tnMapping == LLINEAR){
		s->destSynapse = lGetSynFromAxon(lp->gid);
	}
	else{
	s->destSynapse = getSynapseFromAxon(lp->gid);
	_idT l = LOCAL(lp->gid);

		//tw_printf(TW_LOC, "Axon %i sending message to GID %llu", JSIDE(l), s->destSynapse );



	printf("Axon %i checking in with gid %llu and dest synapse %llu\n ", JSIDE(l), lp->gid, s->destSynapse);

	}
	tw_event *axe = tw_event_new(lp->gid, getNextEventTime(lp), lp);
	Msg_Data *data = (Msg_Data *) tw_event_data(axe);
	data->eventType = AXON_OUT;
	tw_event_send(axe);
	printf("Axon %i checking in with with dest synapse %llu\n", lp->gid, s->destSynapse);
}

void axon_event(axonState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp)
{
  //send a message to the attached synapse
  long rc = lp->rng->count;
  tw_event *axe = tw_event_new(s->destSynapse,getNextEventTime(lp),lp);
  Msg_Data *data = (Msg_Data *) tw_event_data(axe);
  data->eventType = AXON_OUT;


  tw_event_send(axe);
    M->rndCallCount = lp->rng->count - rc;
}


void axon_reverse(axonState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp)
{
	printf("axe reverse \n");
  s->sendMsgCount --;
  long count = M->rndCallCount;
  while (count--) {
      tw_rand_reverse_unif(lp->rng);
  }

}


void axon_final(axonState *s, tw_lp *lp)
{

}




void pre_run()
{

}
