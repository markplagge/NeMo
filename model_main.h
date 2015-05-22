//
// Created by mplagge on 4/28/15.
//
/**
 *  \mainpage True North Timewarp Benchmark Simulation
 *	This is the TNT Benchmarking sim.
 */

#ifndef ROSS_TOP_MODEL_MAIN_H
#define ROSS_TOP_MODEL_MAIN_H

#include "models/neuron_model.h"
#include "models/synapse.h"
#include "assist.h"
#include "ross.h"
#include "spike_generator.h"
#include "mappings.h"
#include "stats_collection.h"
#include <stdio.h>




//synapse enhancement tool


// Variable holders for command lne params & external variables */
/**
 *  Number of neurons per core.
 */
int NEURONS_IN_CORE = 128;
/**
 *  Number of synapses per core.
 */
int SYNAPSES_IN_CORE = 256;
/**
 *  Each PE can have one or more virtual cores running during the simulation. Default is 2.
 */
int CORES_PER_PE = 1;

/** In order to reduce the size of the memory footprint, we're doing < 1 core per PE. This value determines the number of PEs that 
*   are needed to simulate a single core. 
*/ 

int PE_PER_CORE = 2;
/**PES_PER_CORE.PES_PER_COPES_PER_CORE.RE.
 *  Determines the maximum and minimum thresholds for a neuron to fire.
 */
int THRESHOLD_MAX = 100;
/**
 *  Minimum threshold. @see THRESHOLD_MAX
 */
int THRESHOLD_MIN = 30;
/**
 *	Each neuron is connected to the synapses (inputs) within the core it is running in.
 *	These parameters adjust the input weight given to each synapse. */
int SYNAPSE_WEIGHT_MAX = 10;
/** Minimum synapse weight. @see SYNAPSE_WEIGHT_MAX */
int SYNAPSE_WEIGHT_MIN = 1;

tw_stime PER_SYNAPSE_DET_P = .50;

//int DENDRITE_MIN= 1;
//int DENDRITE_MAX= 1;
//int DENDRITE_W_MIN=1;
//int DENDRITE_W_MAX=2;
int CORE_SIZE;
int CORES_IN_SIM;
float CLOCK_SPEED = 1;
//gen lag timer:
tw_stime GEN_LAG = .5;
bool USE_OTHER_LEAKS = false;
bool USE_OTHER_RESET = false;
int MIN_LEAK = 0;
int MAX_LEAK = 10;
char *ALT_LEAK;
char *ALT_RESET;
int MIN_RESET = 0;
int MAX_RESET = 10;
int DEBUG_MODE = 0;
///// Ugly - maybe there's a better way to declare these?
//ROSS OPTIONS:
char *configFilePath;
bool isFile;
tw_stime lookahead = 0;




/** EVENT_BASE - tweakable parameter for memory */
int EVENT_BASE = 100;

//Synapses and neuron max values (for off-by-one errors):
int tt_neurons = 0;
int tt_synapses = 0;
/** Generator Options */
unsigned int GEN_ON = 1;
bool GEN_RND = 1;
int RND_MODE = 0;
unsigned int GEN_PROB = 50;
unsigned int GEN_FCT = 5;
unsigned int GEN_OUTBOUND = 0;
unsigned int GEN_SEL_MODE = 0;
unsigned int SP_DBG = 0;
uint BURST_RATE = 4;


/** Stats variable - number of neruon messages sent. */
stat_t neuronSent = 0;
/** Stats Variable - number of synapse messages sent. */
stat_t synapseSent = 0;
/**Stats counter struct */


//for benchmark, using simplified options.8
const tw_optdef app_opt[] = {
  TWOPT_GROUP("Config File Settings"),
  TWOPT_FLAG("loadF", isFile, "Load a file?"),
  TWOPT_CHAR("cnf_file", configFilePath, "Network Config File Path -- In Network Config format."),
  TWOPT_GROUP("Non-File Configuration"),
  TWOPT_UINT("neurons", NEURONS_IN_CORE, "Neurons per core"),
  TWOPT_UINT("synapses", SYNAPSES_IN_CORE, "Synapses per core"),
  TWOPT_UINT("cores", CORES_PER_PE, "Cores per PE"),
  TWOPT_UINT("cpe", PE_PER_CORE, "Split core simulation amongst this number of PEs"),
  TWOPT_UINT("th_min", THRESHOLD_MIN, "minimum threshold for neurons"),
  TWOPT_UINT("th_max", THRESHOLD_MAX, "maximum threshold for neurons"),
  TWOPT_UINT("wt_min", SYNAPSE_WEIGHT_MIN, "minimum synapse weight"),
  TWOPT_UINT("wt_max", SYNAPSE_WEIGHT_MAX, "maximum synapse wweight"),
  TWOPT_GROUP("Input Sim Generator Options"),
    //TODO: Why does this report "default off"?TWOPT_FLAG("genon", GEN_ON, "Input Generator On"),
  TWOPT_UINT("genon", GEN_ON, "Input Generator On/Off"),
  TWOPT_FLAG("genrd", GEN_RND, "Use Random Input"),
  TWOPT_UINT("rndMd", RND_MODE, "Random gen mode. 0 is GE uniform. 1 is geometric. 2 is binomial. "),
  TWOPT_ULONG("prob", GEN_PROB, "Probability setting"),
  TWOPT_ULONG("ftr", GEN_FCT, "Probability or Lambda for geometric or binomial option."),
  TWOPT_ULONG("genout", GEN_OUTBOUND,
			  "Number of outbound connections for generator (Set <= number of synapses per core.) If set to 0, the generator will attach to all syapses in it's core."),
  TWOPT_STIME("genlag", GEN_LAG, "Lag time for the generator (generator sends a spike once per N"),
    TWOPT_UINT("genSmd", GEN_SEL_MODE, "Generator output selection mode. 0 = sequental, 1 = random."),
  TWOPT_GROUP("Misc. Settings"),
  TWOPT_UINT("burst", BURST_RATE, "Burst rate of synapses (messages per tick sent during synapse activation)"),
  TWOPT_FLAG("debug", DEBUG_MODE, "Enable debug output"),
  TWOPT_FLAG("spd", SP_DBG, "SpecialDebug"),
  TWOPT_STIME("lh", lookahead, "Lookahead Setting"),
  {TWOPT_END()}
};

extern tw_lptype model_lps[];

void pre_run();

//Function headers through the main function.
void neuron_event(neuronState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp);

void synapse_event(synapseState *s, tw_bf *, Msg_Data *M, tw_lp *lp);

void neuron_init(neuronState *s, tw_lp *lp);

void synapse_init(synapseState *s, tw_lp *lp);


//helper functions for expansion of functionality
void setSynapseWeight(neuronState *s, tw_lp *lp, int synapseID);


//reverse functions:
void neuron_reverse(neuronState *, tw_bf *, Msg_Data *, tw_lp *);

void synapse_reverse(synapseState *, tw_bf *, Msg_Data *, tw_lp *);


//neuron management functions
void synapseIn(int synapseID, neuronState *s);

void calculateThreshold(int synapseID, neuronState *s);

void checkFire(int synapseID, neuronState *s);

//functions for the end of the simulation:
void neuron_final(neuronState *s, tw_lp *lp);

void synapse_final(synapseState *s, tw_lp *lp);

//functions for input simulation/handling:
void gen_init(spikeGenState *gen_state, tw_lp *lp);

void gen_pre(spikeGenState *gen_state, tw_lp *lp);

void gen_event(spikeGenState *gen_state, tw_lp *lp, Msg_Data *m);

void gen_reverse(spikeGenState *gen_state, tw_lp *lp,Msg_Data *m);

void gen_final(spikeGenState *gen_state, tw_lp *lp);


/** neuron init helper functions: */
void initRandomWts(neuronState *s, tw_lp *lp);

void initRandomRecurrance(neuronState *s);

void setNeuronThreshold(neuronState *s, tw_lp *lp);

void initNeruonWithMap(neuronState *s, tw_lp *lp, tw_pe *pe);

void initSynapseWithMap(neuronState *s, tw_lp *lp, tw_pe *pe);

/**
 *  typeMapping - custom type mapping function. Uses the mapping functions
 *	defined in mapping.h.
 *
 *
 *  @param gid The global ID of the lp
 *
 *  @return returns the typeID
 */
tw_lpid typeMapping(tw_lpid gid);

#endif //ROSS_TOP_MODEL_MAIN_H
