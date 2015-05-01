// The C main file for a ROSS model
// This file includes:
// - command line argument setup
// - a main function

// includes

#include "model_main.h"

// add your command line opts

tw_lptype model_lps[] = {

    {

        (init_f)neuron_init,
        (pre_run_f)pre_run,
        (event_f)neuron_event,
        (revent_f)neuron_reverse,
        (final_f)neuron_final,
        (map_f)mapping,  // TODO: Check with Elsa about setting this to null for
        // linear mapping.
        sizeof(neuronState)},
    {

        (init_f)synapse_init,
        (pre_run_f)pre_run,
        (event_f)synapse_event,
        (revent_f)synapse_reverse,
        (final_f)synapse_final,
        (map_f)mapping,
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
  s->neuronID = local;
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
  s->reverseLeak = &revNoLeak;
	s->doReset = &resetZero;
  if (isFile ==
      false) {  // no file map, so we use random values. For benchmark,
                // we have to create
    // a recurrance network.
    initRandomWts(s, lp);
    s->dendriteCore =
        tw_rand_integer(lp->rng, 0, g_tw_nkp);  // the dest core can be
    // anywhere from 0 to the number of cores
    // in the simulation, or nkp.

    // todo: Is +1 and -1 the right offset here? I can't think about it right
    // now.
    s->dendriteLocalDest =
        tw_rand_integer(lp->rng, NEURONS_IN_CORE + 1, SYNAPSES_IN_CORE - 1);
    s->dendriteDest = globalID(s->dendriteCore, s->dendriteLocalDest);

  }
  // using a sqlite mapping
  else {
    // initNeruonWithMap(s, lp, lp->pe);
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
    s->perSynapseDet[j] = true;
    s->perSynapseWeight[j] = tw_rand_integer(lp->rng, 0, SYNAPSE_WEIGHT_MAX);
  }
}
/** initNeuronWithMap -- Initializes this particular neuron based on the sqllite
 *map.
 *	Follows a standard mapping layout defined in another file. Sqlite3 is
 *used for performance,
 *	however, JSON might be implemented at a later date. Mappings are based
 *on the
 *  number of cores simulated per PE,  and the number of neurons and synapses in
 *	each core, as defined in #CORES_PER_PE NEURONS_IN_CORE  and
 *#SYNAPSES_IN_CORE.
 *	The specifics for the file
 *  and DB specs are/will be defined in  mapping_specifications.rtf. @see
 *mapping_specifications.rtf */
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
 *random
 *	value based on the parameters #THRESHOLD_MIN and #THRESHOLD_MAX. */
void setNeuronThreshold(neuronState* s, tw_lp* lp) {
  if (isFile) {
    // todo: implement this sql/json/whatever file system
  } else {
    s->threshold = tw_rand_integer(lp->rng, THRESHOLD_MIN, THRESHOLD_MAX);
  }
}

//******************Neuron Functions***********************//
void neuron_event(neuronState* s, tw_bf* CV, Msg_Data* m, tw_lp* lp) {
  tw_lpid self = lp->gid;

  if (DEBUG_MODE == 1)
    printf("Neuron %i recvd synapse spike from %i.\n", s->neuronID,
           m->senderLocalID);
  bool didFire = neuronReceiveMessage(s, tw_now(lp), m, lp);
  // create message if didFire happened:
  if (DEBUG_MODE == 1 && didFire == true)
    printf("Neuron %u has fired. \n", s->neuronID);
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

    tw_event_send(neuronEvent);
  }
}

void neuron_reverse(neuronState* s, tw_bf* CV, Msg_Data* M, tw_lp* lp) {
  // reverse neuron state function
  // do functions in reverse order:
  // 1. Reset State reverse
  s->reverseReset(s);
  // 2. Fire reverse
  s->reverseLeak(s, tw_now(lp));
  // 3. Leak Reverse:

  if (DEBUG_MODE == true)
    printf("Neuron Reverse Event Run \n");
}
void neuron_final(neuronState* s, tw_lp* lp) {
  if (s->fireCount != 0)
    if (DEBUG_MODE == 1)
      printf("Neuron %i:%i has fired %u times. \n", s->coreID, s->neuronID,
             s->fireCount);
  neuronSent += s->fireCount;
}

//******************Synapse Functions***********************//
void synapse_init(synapseState* s, tw_lp* lp) {
  tw_lpid self = lp->gid;

  s->dests = tw_calloc(TW_LOC, "Synapse", sizeof(tw_lpid), NEURONS_IN_CORE);
  synapseSent++;
  /**A synapse location/ID consists of a core number and a synapse number. See
   Mapping Hints for more info,
   accuracy check. */
	regid_t core, loc;
	getLocalIDs(lp->gid, &core, &loc);
  s->coreID =  core;  // TODO: check multi-core function
  s->synID = NEURONS_IN_CORE - loc;

  // set up destination GIDs
  for (regid_t i = 0; i < NEURONS_IN_CORE; i++) {
    s->dests[i] = globalID(s->coreID, i);
  }

  // Spike Generator init setup.
  if (s->synID == 0 && s->coreID == 0) {
    s->spikeGen = tw_calloc(TW_LOC, "Synapse_Init", sizeof(spikeGenState), 1);
    gen_init(s->spikeGen, lp);
  }
}
void synapse_event(synapseState* s, tw_bf* CV, Msg_Data* M, tw_lp* lp) {
  // call synapse helper functions and send messages to all of the neurons in
  // the core here.

  tw_stime ts;
  tw_event* newEvent;
  Msg_Data* data;
  if (M->type == SPKGN && s->spikeGen != NULL) {  // spike generator message
                                                  // data
    gen_event(s->spikeGen, lp);
  }
  // If this is not a spike generator message, then we will proceed according to
  // plan.
  else {
    if (DEBUG_MODE == 1) {
      if (M->type == NEURON)
        printf("Synapse %i firing. Recvd Msg from Neuron %i\n", s->synID,
               M->senderLocalID);
      //}
      if (M->type == INIT)
        printf("Synapse %i init msg received.\n", s->synID);
    }

    for (int i = 0; i < NEURONS_IN_CORE; i++) {
      // ts = tw_rand_exponential(lp->rng,4);
      ts = getNextEventTime(lp);  // function calls.
      newEvent = tw_event_new(s->dests[i], ts, lp);
      data = (Msg_Data*)tw_event_data(newEvent);
      data->senderLocalID = s->synID;
      // data->destCore = coreID(s->dests[i]); //TODO: Add support for more than
      // one core.
      data->sourceCore = s->coreID;
      data->type = SYNAPSE;
      // event:

      if (DEBUG_MODE == 1) {
        printf("Sending message to GID %lu, at time: %f.\n", s->dests[i], ts);
      }
      tw_event_send(newEvent);

      // tw_event *e = tw_event_new(s->dests[i], nextTime, lp);
      // tw_event_send(e);
    }
  }
}
void synapse_reverse(neuronState* s, tw_bf* CV, Msg_Data* M, tw_lp* lp) {
  // synapse event reverse function goes here -  please ensure that the code
  // does not reverse into neurons?
  // TODO: double check that thyis is hwo we would handle synaptic reversal
}
void synapse_final(synapseState* s, tw_lp* lp) {
}

/* Spike Generator Functions */
void gen_init(spikeGenState* gen_state, tw_lp* lp) {
  // This is a spike generator init - it was used as a unique LP, but a single
  // synapse now holds the generator/IO system.
  gen_state->outboud = GEN_OUTBOUND;
  // probability settings. Could move within if statement, but nah.
  gen_state->rndSpikes.randomRate = GEN_PROB / 100;
  gen_state->rndSpikes.rndFctVal = GEN_FCT / 100;
  if (GEN_RND == true) {
    gen_state->rndSpikes.randomRate = GEN_PROB;
    gen_state->rndSpikes.rndFctVal = GEN_FCT;
    gen_state->rndSpikes.randMethod = RND_MODE;
    // randomized values for hooking this machine up to the various neurons.

    // rand_uniform function tw_rand_unif(lp->rng)
    gen_state->rndSpikes.randMethod = UNF;
    gen_state->spikeGen = uniformGen;
  }
  // set up the outbound connections.
  gen_state->connectedSynapses =
      tw_calloc(TW_LOC, "Synapse", sizeof(tw_lpid*), GEN_OUTBOUND);

  long totalSyns = getTotalSynapses();
  printf("Synapses in total sim: %ld\n", totalSyns);
  for (int i = 0; i < GEN_OUTBOUND; i++) {
    regid_t core, local;
    core = tw_rand_integer(lp->rng, 0, g_tw_nkp);
    local = NEURONS_IN_CORE + ( tw_rand_integer(lp->rng, 0, getTotalSynapses()) % NEURONS_IN_CORE);
    gen_state->connectedSynapses[i] = globalID(core, local);
  }
  // Initial event seeding:
  tw_stime ts = getNextEventTime(lp) + GEN_LAG;  // function calls.
  tw_event* newEvent = tw_event_new(lp->gid, ts + GEN_LAG, lp);
  Msg_Data* data = (Msg_Data*)tw_event_data(newEvent);
  data->type = SPKGN;
  tw_event_send(newEvent);
}
void gen_pre(spikeGenState* gen_state, tw_lp* lp) {
}
void gen_event(spikeGenState* gen_state, tw_lp* lp) {
  // We have event - Check if we should send a spike to the next synapse:

  // while (gen_state->spikeGen(gen_state,lp))
  //{
  bool willFire = gen_state->spikeGen(gen_state, lp);

  tw_lpid dest;
  tw_stime ts;
  if (willFire) {
    // TODO: enable sequental mode
    ts = getNextEventTime(lp);
    switch (gen_state->genMode) {
      case UNF:
        dest = gen_state->connectedSynapses[tw_rand_integer(
            lp->rng, 0, gen_state->outboud)];
        break;

      default:
        dest = 0;
        break;
    }
    // set up DEST here - items are stored as the absolute value of the synapse,
    // need to convert to GID.
    dest = ((NEURONS_IN_CORE * ((dest / CORE_SIZE) + 1)) + dest) - 1;
    tw_event* newEvent = tw_event_new(dest, ts, lp);
    Msg_Data* data = (Msg_Data*)tw_event_data(newEvent);
    data->type = SYNAPSE;
    tw_event_send(newEvent);
  }
  // Reprime the generator:
  //}
  tw_event* prime = tw_event_new(lp->gid, getNextEventTime(lp), lp);
  Msg_Data* newData = (Msg_Data*)tw_event_data(prime);
  newData->type = SPKGN;
  tw_event_send(prime);
}
void gen_reverse(spikeGenState* gen_state, tw_lp* lp) {
}
void gen_final(spikeGenState* gen_state, tw_lp* lp) {
}


///////////////MAIN///////////////////////
int model_main(int argc, char* argv[]) {
  int i;
  int num_lps_per_pe;
  CORE_SIZE = NEURONS_IN_CORE + SYNAPSES_IN_CORE;
  // tw_opt_add(app_opt);
  g_tw_nkp = CORES_PER_PE;
  tw_init(&argc, &argv);
  initMapVars(NEURONS_IN_CORE,SYNAPSES_IN_CORE,CORES_PER_PE);
  g_tw_events_per_pe = 2048 * CORE_SIZE * CORES_PER_PE;

  // Trying linear mapping first - it basically is linear.
  // g_tw_mapping = LINEAR;

  // initial_mapping();
  g_tw_mapping = CUSTOM;
  g_tw_custom_initial_mapping = &initial_mapping;
  g_tw_custom_lp_global_to_local_map = &mapping_to_local;
  tw_define_lps(CORE_SIZE * CORES_PER_PE, sizeof(Msg_Data), 0);


  // Useful ROSS variables and functions
  // tw_nnodes() : number of nodes/processors defined
  // g_tw_mynode : my node/processor id (mpi rank)

  // Useful ROSS variables (set from command line)
  // g_tw_events_per_pe
  // g_tw_lookahead
  // g_tw_nlp
  // g_tw_nkp
  // g_tw_synchronization_protocol

  // set up LPs within ROSS

  // g_tw_nlp gets set by tw_define_lps
  /*
    for (i = 0; i < g_tw_nlp; i++) {
      tw_lp_settype(i, &model_lps[0]);
      // calculate my core and my local value:
      regid_t core, local;
      // so, n cores per pe, with x items on them.
      // g_tw_lp[i]->gid;
      core = (g_tw_lp[i]->gid) / CORE_SIZE;
      local = (g_tw_lp[i]->gid % CORE_SIZE);
      printf("Core calculated at %ul \n Local calculated at %ul ", core, local);
  */

  //}

  // Do some file I/O here? on a per-node (not per-LP) basis
  // load mapping here.
  tw_run();

  tw_end();

  return 0;
}
int main(int argc, char* argv[]) {
  tw_opt_add(app_opt);
  int rv = model_main(argc, argv);
  if (g_tw_mynode == 0) {  // && DEBUG_MODE == true){
    printf("Neurons fired %lu messages, and synapses fired %lu messages.",
           neuronSent, synapseSent);
  }
  return rv;
}