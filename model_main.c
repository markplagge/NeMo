// The C main file for a ROSS model
// This file includes:
// - command line argument setup
// - a main function

// includes

#include "model_main.h"
#include "assist.h"

// add your command line opts

tw_lptype model_lps[] = {

    {

     (init_f)neuron_init, (pre_run_f)pre_run, (event_f)neuron_event,
     (revent_f)neuron_reverse, (final_f)neuron_final,
     (map_f)mapping,  // TODO: Check with Elsa about setting this to null for
     // linear mapping.
     sizeof(neuronState)},
    {

     (init_f)synapse_init, (pre_run_f)pre_run, (event_f)synapse_event,
     (revent_f)synapse_reverse, (final_f)synapse_final, (map_f)mapping,
     sizeof(synapseState)},
    {0}

};

void pre_run() {

}  // prerun function.
                   // TODO: Add pre-run stuff - not sure if we need anything.

/*********************************************************************************************'
 * Main function definitions. Based on the prototype tnt_main
 * this model implements the neuromorphic baseline benchmark.
 * */
void neuron_init(neuronState* s, tw_lp* lp) {
  tw_lpid self = lp->gid;

  // set neuron local id
  regid_t core, local;

  getLocalIDs(self, &core, &local);
  s->coreID = core;

  s->neuronID = getNeuronID(self);  // local;

  // initial membrane potential values
  s->prVoltage = 0;
  s->fireCount = 0;
  s->lastLeakTime = 0;
  s->lastActiveTime = 0;
  // benchmarking default values.
  // benchmark leak means membrane potential goes to zero after firing, and no
  // leaks
  s->leakRate = 0;
  s->leak = &noLeak;
  s->lastLeakTime = 0;
  s->doReset = &resetZero;
  s->reverseLeak = &revNoLeak;
  s->reverseReset = &resetZero;
	s->integrationCount = 0;
	s->burstRate = BURST_RATE;
	s->neuronsInCore = NEURONS_IN_CORE;
  if (isFile ==
      false) {  // no file map, so we use random values. For benchmark,
    // we have to create
    // a recurrance network.
    initRandomWts(s, lp);
    s->dendriteCore =
        tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);  // the dest core can be
    // anywhere from 0 to the number of cores
    // in the simulation, or nkp.

    // todo: Is +1 and -1 the right offset here? I can't think about it right
    // now.
    s->dendriteLocalDest =
        tw_rand_integer(lp->rng, NEURONS_IN_CORE, SYNAPSES_IN_CORE);
    s->dendriteDest = globalID(s->dendriteCore, s->dendriteLocalDest);
    setNeuronThreshold(s, lp);

  }
  // using a sqlite mapping
  else {
    // initNeruonWithMap(s, lp, lp->pe);
  }
  if (DEBUG_MODE == true) {
    startRecord();
    mapRecord(typeMapping(lp->gid), "Neuron", s->neuronID, s->coreID, lp->id,
              lp->gid);
    recordNeuron(s);
    endRecord();
  }
}

/** initRdmWts - initializes random weights for synapses. Essentially creates a
 * randomized neural network if
 * called on all new neurons.
 * @param *s The new neuron, created by ROSS indirectly.
 * */
void initRandomWts(neuronState* s, tw_lp* lp) {
  s->perSynapseDet = tw_calloc("Error-SynapseWeightInit", 81, "Neurons",
                               sizeof(bool), SYNAPSES_IN_CORE);
  s->perSynapseWeight = tw_calloc("Error-SynapseWeightInit", 81, "Neurons",
                                  sizeof(_neVoltType), SYNAPSES_IN_CORE);
  // randomized wts:
  for (int j = 0; j < SYNAPSES_IN_CORE; j++) {
      //I really want to use some fancier random here. TODO: add one
    s->perSynapseDet[j] = tw_rand_unif(lp->rng) <= PER_SYNAPSE_DET_P;
    s->perSynapseWeight[j] = tw_rand_integer(lp->rng, SYNAPSE_WEIGHT_MIN, SYNAPSE_WEIGHT_MAX);
    s->sgnLambda = tw_rand_integer(lp->rng,1,10) > 5 ? -1 : 1;

  }
}
/** initNeuronWithMap -- Initializes this particular neuron based on the sqllite
 * map.
 *	Follows a standard mapping layout defined in another file. Sqlite3 is
 * used for performance,
 *	however, JSON might be implemented at a later date. Mappings are based
 * on the
 *  number of cores simulated per PE,  and the number of neurons and synapses in
 *	each core, as defined in #CORES_PER_PE NEURONS_IN_CORE  and
 ***#SYNAPSES_IN_CORE.
 *	The specifics for the file
 *  and DB specs are/will be defined in  mapping_specifications.rtf. @see
 * mapping_specifications.rtf */
void initNeuronWithMap(neuronState* s, tw_lp* lp, tw_pe* pe) {
  // nothing yet
  printf("Not implemented yet");
}
/** initSynapseWithMap -- Initializes this particular synapse based on the
 * sqllite map. See #initNeuronWithMap() for more information. */
void initSynapseWithMap(neuronState* s, tw_lp* lp, tw_pe* pe) {
  printf("not implemented yet");
}

//*************************************************//
// Neuron threshold functions //

/** setNeuronThreshold - Sets the threshold value of the neuron. If there is a
 *	map, then this will use the map's values. Otherwise, it sets it to a
 * random
 *	value based on the parameters #THRESHOLD_MIN and #THRESHOLD_MAX. */
void setNeuronThreshold(neuronState* s, tw_lp* lp) {
  if (isFile) {
    // todo: implement this sql/json/whatever file system
  } else
    s->threshold = tw_rand_integer(lp->rng, THRESHOLD_MIN, THRESHOLD_MAX);
}

//****************Neuron Event Functions ************************//
//TODO: Add a time wait for sending.
void neuron_event(neuronState* s, tw_bf* CV, Msg_Data* m, tw_lp* lp) {
  if(m->type == NEURON) {
    if(DEBUG_MODE == true)
         recordOutOfBounds("neuronOut", s->dendriteLocalDest,s->dendriteCore,s->dendriteDest,s->coreID, s->neuronID, lp->gid);

  }  else{
      long startCount = lp->rng->count;
      tw_lpid self = lp->gid;

      neuronSent++;
      tw_lpid d = s->dendriteDest;
      // if (DEBUG_MODE == 1)
      // tw_lpid d;
      // printf("Neuron %i recvd synapse spike from %i.\n",
      // s->neuronID,m->senderLocalID);

	  bool didFire = false;
	  didFire = neuronReceiveMessage(s, tw_now(lp), m, lp);
      // create message if didFire happened:
      // if (DEBUG_MODE == 1 && didFire == true)
      // printf("Neuron %u has fired. \n", s->neuronID);



      if (didFire == true) {
        tw_event* neuronEvent;
        Msg_Data* data;
        // added these separately to make it easier to change
        tw_lpid dest = s->dendriteDest;

        tw_stime ts = getNextEventTime(lp);  // tw_rand_exponential(lp->rng, 4) /
        // 10;
        neuronEvent = tw_event_new(dest, ts, lp);

        data = (Msg_Data*)tw_event_data(neuronEvent);
        data->destCore = s->dendriteCore;
        data->destLocalID = s->dendriteLocalDest;
        data->sender = self;
        data->senderLocalID = s->neuronID;
        data->sourceCore = s->coreID;
        data->type = NEURON;
        data->partial = -1;
        tw_event_send(neuronEvent);
        if (DEBUG_MODE == true) d = dest;
      }
      if (DEBUG_MODE == true) {
        startRecord();
        // todo: This is a potential memory leak - need to somehow free this but
        // only after it is done being used.

        char* evt = sqlite3_mprintf(
            "Send? %i --- Neuron States-  FireCount: %i, Last Active: %f, Last "
            "Leak: %i -- from synapse GID: %llu",
            didFire, s->fireCount, s->lastActiveTime, s->lastLeakTime, m->sender);
        neuronEventRecord(lp->gid, s->coreID, s->neuronID, getSynapseID(d),
                          tw_now(lp), s->prVoltage, evt);
        //TODO: Remove this for actual testing. Only works for one core per pe. 
        if(s->dendriteLocalDest < NEURONS_IN_CORE && CORES_PER_PE == 1) { 
          recordOutOfBounds("neuronOut", s->dendriteLocalDest,s->dendriteCore,s->dendriteDest,s->coreID, s->neuronID, lp->gid);

        }
        endRecord();
      }

      m->rndCallCount = lp->rng->count - startCount;
    }
}

void neuron_reverse(neuronState* s, tw_bf* CV, Msg_Data* M, tw_lp* lp) {
  neuronReverseFinal(s, CV, M, lp);

  // Log the reverse event if debug mode is on.
  // TODO: Switch the string literal with a dynamically allocated string, so
  // that whenever the memory leak is fixed from the other event, this won't
  // cause a segfault.
  if (DEBUG_MODE == true)
    neuronEventRecord(lp->gid, s->coreID, s->neuronID, M->senderLocalID,
                      tw_now(lp), s->cVoltage, "NeuronReverse");
}
void neuron_final(neuronState* s, tw_lp* lp) {
	stats->integrationCount += s->integrationCount;
	stats->neuronFireCount += s->fireCount;
}

//******************Synapse Functions***********************//
void synapse_init(synapseState* s, tw_lp* lp) {
  tw_lpid self = lp->gid;

  s->fireCount = 0;
  s->dests = tw_calloc(TW_LOC, "Synapse", sizeof(tw_lpid), NEURONS_IN_CORE);

  /**A synapse location/ID consists of a core number and a synapse number. See
     Mapping Hints for more info,
     accuracy check. */
  regid_t core, loc;
  getLocalIDs(lp->gid, &core, &loc);
  s->coreID = core;  // TODO: check multi-core function
  s->synID = loc;
  tw_lpid arrrgs = lp->gid;
  // set up destination GIDs
  for (regid_t i = 0; i < NEURONS_IN_CORE; i++) {
    s->dests[i] = globalID(s->coreID, i);
    tw_lpid x = s->dests[i];
    x = x;
  }

  // Spike Generator init setup.
  if (s->synID - NEURONS_IN_CORE == 0) {
    s->spikeGen = tw_calloc(TW_LOC, "Synapse_Init", sizeof(spikeGenState), 1);
    gen_init(s->spikeGen, lp);
    // Added to make sure generators are being created properly. NOt really
    // needed, but nice to know.
    if (DEBUG_MODE == true) {
      startRecord();
      mapRecord(typeMapping(lp->gid), "Synapse Generator", s->synID, s->coreID,
                lp->id, lp->gid);
      endRecord();
    }
  }
  if (DEBUG_MODE == true) {
    startRecord();
    mapRecord(typeMapping(lp->gid), "Synapse", s->synID, s->coreID, lp->id,
              lp->gid);
    endRecord();
  }
}
void synapse_event(synapseState* s, tw_bf* CV, Msg_Data* M, tw_lp* lp) {
  // call synapse helper functions and send messages to all of the neurons in
  // the core here.
  long startCount = lp->rng->count;
  tw_stime ts;
  tw_event* newEvent;
  Msg_Data* data;

  if (M->type == SPKGN && s->spikeGen != NULL){  // spike generator message
    gen_event(s->spikeGen, lp, M);
  }
  else {
    s->fireCount++;
    if (DEBUG_MODE == 1) {
      startRecord();
    }

    /* trying an optimization code here.
       for (int i = 0; i < NEURONS_IN_CORE; i++) {
       // ts = tw_rand_exponential(lp->rng,4);
       ts = getNextEventTime(lp);  // function calls.
       newEvent = tw_event_new(s->dests[i], ts, lp);
       data = (Msg_Data*)tw_event_data(newEvent);
       data->senderLocalID = s->synID;
       // data->destCore = coreID(s->dests[i]); //TODO: Add support for more
       than
       // one core.
       data->sourceCore = s->coreID;
       data->type = SYNAPSE;
       // event:
     */
    int_fast32_t part;

    // Synapse->neuyron event sending loop. sends messages to all of the neurons
    // attvched to the synapses core. There is a send limit in order to prevent
    // runaway event generation, BURST_RATE. If a synapse sends BURST_RATE
    // number of messages
    // but has not sent messages to all of the neruons in its core, it will send
    // a synapse message to itself letting it know where to resume. (part &
    // partial)
    // messages from neurons are handled first

    if (M->type == NEURON) {  // mesage from neuron.
      ts = getNextEventTime(lp);
      newEvent = tw_event_new(lp->gid, ts, lp);
      data = (Msg_Data*)tw_event_data(newEvent);
      data->senderLocalID = s->synID;
      data->partial = NEURONS_IN_CORE;
      data->type = SYNAPSE;
      tw_event_send(newEvent);

      if (DEBUG_MODE == true)
        synapseEventRecord(lp->gid, s->coreID, s->synID, tw_now(lp), lp->gid);
    } else {
      part = M->partial - 1;
      for (unsigned int fired = 0; fired < BURST_RATE && part >= 0;
           fired++, part--) {
        ts = getNextEventTime(lp);
        newEvent = tw_event_new(s->dests[part], ts, lp);
        data = (Msg_Data*)tw_event_data(newEvent);
        data->senderLocalID = s->synID;
        data->sourceCore = s->coreID;
        data->partial = -1;  // messages from synapse to neuron and neuron to
        // synapse do not use this val.
        data->type = SYNAPSE;  // msg from synapse to neuron are of type SYNAPSE
        tw_event_send(newEvent);
        if (DEBUG_MODE == true){
          synapseEventRecord(lp->gid, s->coreID, s->synID, tw_now(lp),
                             s->dests[part]);
     
        }

      }
      // after running the burst, check to see if there are any remaning neurons
      // to send messages to.
      if (part >= 0) {
        // seed synapse with remaining partial data.
        ts = getNextEventTime(lp);
        newEvent = tw_event_new(lp->gid, ts, lp);
        data = (Msg_Data*)tw_event_data(newEvent);
        data->senderLocalID = s->synID;
        data->partial = part;
        data->destCore = s->coreID;
        data->type = SYNAPSE;  // When sending messages from synapse to self,
        // synapse message type is used.
        tw_event_send(newEvent);
      }
    }
    if (DEBUG_MODE == true) endRecord();
  }
  M->rndCallCount = lp->rng->count - startCount;
}

void synapse_reverse(synapseState* s, tw_bf* CV, Msg_Data* M, tw_lp* lp) {
  // Grand Central Dispacth pattern - reversal function calling here though.
  // TODO: currently, SPIKE init (system init) functions are not handled. Add
  // handlers
  if (M->type == SPKGN && s->spikeGen != NULL) gen_reverse(s->spikeGen, lp, M);
  // reverse the randomized values:
  long count = M->rndCallCount;
  while (count--) tw_rand_reverse_unif(lp->rng);
}
tw_lpid localFire = 0;
void synapse_final(synapseState* s, tw_lp* lp) {
  // TODO: add an MPI_reduce function here to collect synapse stats.
  //	MPI_Reduce(&s->fireCount, &synapseSent, 1, MPI_UNSIGNED_LONG_LONG,
  // MPI_SUM, 0, MPI_COMM_WORLD);
  synapseSent += s->fireCount;
}

/* Spike Generator Functions */
void gen_init(spikeGenState* gen_state, tw_lp* lp) {
  // This is a spike generator init - it was used as a unique LP, but a single
  // synapse now holds the generator/IO system.

  gen_state->currentOut = 0;
  gen_state->outbound = GEN_OUTBOUND;
  // probability settings. Could move within if statement, but nah.
  gen_state->randomRate = GEN_PROB / 100;
  gen_state->rndFctVal = GEN_FCT / 100;
  // RANDOM GENERATOR MAP SETUP

  if (GEN_RND == true && GEN_OUTBOUND == 0) {
    gen_state->randomRate = GEN_PROB;
    gen_state->rndFctVal = GEN_FCT;
    gen_state->randMethod = RND_MODE;
    // randomized values for hooking this machine up to the various neurons.
    // rand_uniform function tw_rand_unif(lp->rng)
    gen_state->randMethod = UNF;
    gen_state->spikeGen = uniformGen;
    // set up the outbound connections.
    long totalSyns = SYNAPSES_IN_CORE * CORES_PER_PE;
    // printf("Synapses in total sim: %ld\n", totalSyns);
    gen_state->outbound = totalSyns;
    // printf("Generator\nCore\tLocal\tGlobal\n");

    gen_state->connectedSynapses =
        tw_calloc(TW_LOC, "Synapse", sizeof(tw_lpid*), gen_state->outbound);

    for (int i = NEURONS_IN_CORE - 1, j = 0; i < totalSyns; i++, j++) {
      tw_lpid gid = 0;
      regid_t core, local;
      int coreOffset = (g_tw_mynode * CORES_PER_PE);
      // WARNING: Monitor this function to ensure it is showing proper values.
      core = i / CORE_SIZE + coreOffset;
      local = i;
			//gid = globalID(core, local);
      gen_state->connectedSynapses[i] = globalID(core, local);
    }
  } else if (GEN_RND == true) {
    gen_state->randomRate = GEN_PROB;
    gen_state->rndFctVal = GEN_FCT;
    gen_state->randMethod = RND_MODE;
    // randomized values for hooking this machine up to the various neurons.

    // rand_uniform function tw_rand_unif(lp->rng)
    switch (RND_MODE) {
      case UNF:
        gen_state->spikeGen = uniformGen;
        break;
      case EXP:
      default:
        gen_state->spikeGen = expGen;
        break;
    }

    // set up the outbound connections.

    gen_state->outbound = GEN_OUTBOUND;
    gen_state->connectedSynapses =
        tw_calloc(TW_LOC, "Synapse", sizeof(tw_lpid*), gen_state->outbound);

    for (int i = 0; i < GEN_OUTBOUND; i++) {
      tw_lpid gid = 0;
      do {
        regid_t core, local;
        core = tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);
        // TODO - double check the +1 and -1 here.
        local =
            tw_rand_integer(lp->rng, NEURONS_IN_CORE + 1, SYNAPSES_IN_CORE - 1);
        gid = globalID(core, local);
        // printf("%lu\t%lu\t%llu\n",core,local,gid);

        gen_state->connectedSynapses[i] = globalID(core, local);

        // printf("\n");
      } while (gid > g_tw_nlp * g_tw_avl_node_count);
    }
  }
  // Here we read the generator setup map
  // TODO: Implement this!

  // Initial event seeding:
  tw_stime ts = getNextEventTime(lp) + GEN_LAG;  // function calls.
  tw_event* newEvent = tw_event_new(lp->gid, ts + GEN_LAG, lp);
  Msg_Data* data = (Msg_Data*)tw_event_data(newEvent);
  data->type = SPKGN;
  tw_event_send(newEvent);
}
void gen_pre(spikeGenState* gen_state, tw_lp* lp) {}
void gen_event(spikeGenState* gen_state, tw_lp* lp, Msg_Data* m) {
  // reverse fn worker

  // We have event - Check if we should send a spike to the next synapse:

  // while (gen_state->spikeGen(gen_state,lp))
  //{
  bool willFire = gen_state->spikeGen(gen_state, lp);

  tw_lpid dest;
  tw_stime ts;

  if (willFire) {
    ts = getNextEventTime(lp);
    switch (gen_state->genMode) {
      case UNF:  // Unified random selection for next element.
        dest = gen_state->connectedSynapses[tw_rand_integer(
            lp->rng, 0, gen_state->outbound - 1)];
        break;
      case SELECT:
        dest = gen_state->connectedSynapses[gen_state->currentOut];
        gen_state->currentOut++;  // mmm distributed loops
        gen_state->currentOut = gen_state->currentOut % gen_state->outbound;
        break;
      default:
        dest = 0;
        break;
    }
    // set up DEST here - items are stored as the absolute value of the synapse,

    tw_event* newEvent = tw_event_new(dest, ts, lp);
    Msg_Data* data = (Msg_Data*)tw_event_data(newEvent);
    data->type = NEURON;
    data->sender = 0;
    data->destCore = 0;
    data->destLocalID = 0;
    data->sourceCore = 0;
    data->prevVoltage = 0;
    data->senderLocalID = 0;
    // This is set to number of neurons in the core, since the default behavior
    // is to
    // have a synapse send a message to the n-1th neuron upon receipt of a
    // synapse message.
    data->partial = NEURONS_IN_CORE;

    // tw_printf("SENDING SEED EVENT - CAUSING ISSUES Dest GID %llu -- Dest LP
    // (reported) %llu \n", dest, newEvent->dest_lp->gid );
    tw_event_send(newEvent);
  }
  // Reprime the generator:
  //}
  tw_event* prime = tw_event_new(lp->gid, getNextEventTime(lp), lp);
  Msg_Data* newData = (Msg_Data*)tw_event_data(prime);
  newData->type = SPKGN;
  tw_event_send(prime);
}
void gen_reverse(spikeGenState* gen_state, tw_lp* lp, Msg_Data* m) {
  if (gen_state->selectMode == SELECT) gen_state->currentOut--;
}
void gen_final(spikeGenState* gen_state, tw_lp* lp) {}

////////////////////TypeMapping///////////
tw_lpid typeMapping(tw_lpid gid) {
  regid_t localID;
  regid_t coreID;
  //	getLocalIDs(gid, &coreID, &localID);
  // if the localID is > than the number of neurons, this is a synapse.
  int id;

  getLocalIDs(gid, &coreID, &localID);
  id = localID % CORE_SIZE;
  if (g_tw_mynode > 0 && DEBUG_MODE == true)
    printf("Mapping -- local id is %u, id calculated to be %u\n", localID, id);
  id = id < NEURONS_IN_CORE ? 0 : 1;
  //	getLocalIDs(gid, &coreID, &localID);
  //	localID = localID - (coreID * CORE_SIZE);
  //	id = localID < NEURONS_IN_CORE ? 0 :1;
  //	if(coreID == 0){
  //		id = localID < NEURONS_IN_CORE  ? 0:1;
  //	}
  //	else
  //		id = localID - (SYNAPSES_IN_CORE + (NEURONS_IN_CORE * coreID)) ?
  // 0
  //: 1;
  return id;  // TODO: Switch this to an enum
}
///////////////MAIN///////////////////////
// TODO: Check memory allocation scheme - ensure it makes sense!!
int model_main(int argc, char* argv[]) {



  tw_init(&argc, &argv);  // toto
  CORE_SIZE = NEURONS_IN_CORE + SYNAPSES_IN_CORE;
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  CORES_IN_SIM = CORES_PER_PE * world_size;
  int num_lps_per_pe = (CORE_SIZE * (CORES_PER_PE));
  ;

  // tw_opt_add(app_opt);

  if (DEBUG_MODE == true) {
    initDB();
    printf("Init db call \n");
  }
  g_tw_events_per_pe = EVENT_BASE * CORE_SIZE * CORES_PER_PE;

  // Trying linear mapping first  - it basically is linear.
  // g_tw_mapping = LINEAR;

  // initial_mapping();
  g_tw_mapping = CUSTOM;
  g_tw_custom_initial_mapping = &initial_mapping;
  g_tw_custom_lp_global_to_local_map = &mapping_to_local;
  g_tw_lp_typemap = &typeMapping;
  g_tw_lp_types = model_lps;
  g_tw_lookahead = lookahead;
  tw_define_lps(num_lps_per_pe, sizeof(Msg_Data), 0);
  // set the types:
  tw_lp_setup_types();

  tw_run();

  tw_end();
  if (DEBUG_MODE == 1) finalClose();
  // trying all reduce here:
  tw_lpid totalSyn;

  return 0;
}

int main(int argc, char* argv[]) {
	stats = malloc(sizeof(localStat*));
	stats->integrationCount = 0;
	stats->neuronFireCount = 0;

  tw_opt_add(app_opt);
  int rv = model_main(argc, argv);
  if (g_tw_mynode == 0)  // && DEBUG_MODE == true){
    printf("Neurons integrated %lu times, and synapses fired %lu messages.\n",
           neuronSent, synapseSent);
	printf("***Official Stats - Neurons integrated %lu times and fired %lu times. \n", stats->integrationCount, stats->neuronFireCount);
  return rv;
}
