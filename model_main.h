//
//  model_main.h
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/17/15.
//
//

#ifndef __ROSS_TOP__model_main__
#define __ROSS_TOP__model_main__

#include <stdio.h>
#include "assist.h"
#include "ross.h"
#include "models/axon.h"
#include "models/neuron.h"
#include "models/synapse.h"
#include "input_simulator.h"
#include "mapping.h"
//#include "clMapping.h"
#include "neuron_out_stats.h"
#include <math.h>

#include "bioModel.h"

#include <stdbool.h>

#define nlog s->log 
#define newlog (neuEvtLog*)calloc(sizeof(neuEvtLog),1);


extern int n_created ;
extern int s_created ;
extern int a_created ;
        // Variable holders for command lne params & external variables

/**
 *  Number of neurons per core.
 */
int NEURONS_IN_CORE = 256;
/** number of synapses per core. Calculated value, needs to be neurons * axons */
int SYNAPSES_IN_CORE;
/** Number of axions per core. Generally is set to 1-1 with neurons in core */
int AXONS_IN_CORE;
/* Given number of cores in simulation */
unsigned int CORES_IN_SIM = 1;

/// Memory Tuning
int eventAlloc = 2;
unsigned int GEN_ON = 1;///< Is the input generator enabled?

bool GEN_RND = 1; //!< Generator random mode flag
//unsigned int RND_MODE = 0;
//unsigned int GEN_PROB = 50;
//unsigned int GEN_FCT = 5;
//unsigned int GEN_OUTBOUND = 0;
//unsigned int GEN_SEL_MODE = 0;
//unsigned int SP_DBG = 0;
unsigned long LPS_PER_PE;
unsigned long SIM_SIZE;
unsigned long LP_PER_KP;
tw_stime LH_VAL = 0.001;
unsigned int RAND_WT_PROB = 2;
bool DEBUG_MODE = 0;
bool BASIC_SOP = false;
bool TW_DELTA = false;
bool BULK_MODE = false;
bool PHAS_VAL = false;
bool TONIC_SPK_VAL = false;
bool TONIC_BURST_VAL = false;
bool DEPOLAR_VAL = false;
bool SAVE_MEMBRANE_POTS = false;
bool SAVE_SPIKE_EVTS = false;
bool SAVE_NEURON_OUTS = false;
 stat_type neuronSOPS = 0;
 stat_type synapseEvents = 0;
bool validation;

stat_type fireCount;
/** littleTick is the resolution of little ticks 
* (events between neuron fire events) */
tw_stime littleTick = .001;
/** changes random time parameter */
tw_stime CLOCK_RANDOM_ADJ = 1.0;
/** selects different random modes:*/
timeRandomSel CLOCK_RND_MODE = RND_EXP;

extern tw_lpid* myGIDs;

/* Mapping values */
mapTypes tnMapping;
	//tw_lptype model_lps[];
/**  Determines the maximum thresholds for a neuron to fire.
 */
thresh_type THRESHOLD_MAX = 2;

 thresh_type NEG_THRESHOLD_MAX = 2;
/**
 *  Minimum threshold. @see THRESHOLD_MAX
 */
thresh_type THRESHOLD_MIN = 1;

thresh_type NEG_THRESHOLD_MIN = 1;

int NEG_THRESH_SIGN = -1;


volt_type RESET_VOLTAGE_MAX = 1;
volt_type RESET_VOLTAGE_MIN = -1;

thresh_type RAND_RANGE_MIN = 1;
thresh_type RAND_RANGE_MAX = 10;


stat_type totalSOPS;
stat_type totalSynapses;


FILE *neuronWT; //neuron weight table file handle
FILE *neuronOT; //neuron output table file handle


//Stats & Stats related byproducts (including stats structs)
struct supernStats {
    unsigned int npe;
    unsigned long long SOP;
    unsigned long long neuronSpikes;
    unsigned long long totalSynapseMsgs;
    tw_stime runtime;
    tw_stime totalTime;
    
    
};

tw_statistics statsOut();
int csv_model_stats(tw_statistics s);
int write_csv(struct supernStats *stats, char const *fileName);

//
/**
 *	Each neuron is connected to the synapses (inputs) within the core it is running in.
 *	These parameters adjust the input weight given to each synapse. */
int32_t SYNAPSE_WEIGHT_MAX = 1;
/** Minimum synapse weight. @see SYNAPSE_WEIGHT_MAX */
int32_t SYNAPSE_WEIGHT_MIN = 0;

tw_stime PER_SYNAPSE_DET_P = .50;


	//Simulation Variables
/**CORE_SIZE is equal to the number of axions * number of aneurons + num neurons + num axions */
int CORE_SIZE;

/* **** Model Main Function */
/**
 * @brief createLPs will create the LPs needed for the simulation on this PE.
 * Call once during model init.
 */
void createLPs();
/**
 * @brief pre_run Not used - placeholder function for compatibility.
 * In future revisions, this is where file IO will occur.
 *
 */
void pre_run();

/**
 * @brief neuron_init Neuron lp initialization.
 * @param s
 * @param lp
 */
void neuron_init(neuronState *s, tw_lp *lp);
/**
 * @brief setSynapseWeight is called from \a neuron_init and sets up the weights
 * of a neuron. Keeping it as a seperate function. Eventually will read in file map data.
 * @param s
 * @param lp
 * @param synapseID
 */

//Command line options:
int B_TH_MIN = 1;
int B_TH_MAX = 100;
int B_P_AX_WT_MAX = 100;
int B_P_AX_WT_MIN = 0;
int B_N_AX_WT_MAX = -10;
bool B_LEAK_ON = true;
int B_LEAK_WEIGHT = -10;
int B_NEG_THRESHOLD = -10;
tw_stime B_CROSSBAR_PROB = .5;
tw_stime B_EXITE_PROB = .75;

const tw_optdef app_opt[]= {
  TWOPT_GROUP("Randomized Neuron Parameters"),
  TWOPT_UINT("th_min", THRESHOLD_MIN, "minimum threshold for neurons"),
    TWOPT_UINT("th_max", THRESHOLD_MAX, "maximum threshold for neurons"),
    TWOPT_UINT("wt_min", SYNAPSE_WEIGHT_MIN, "minimum synapse weight -- is treated as 0-val"),
    TWOPT_UINT("wt_max", SYNAPSE_WEIGHT_MAX, "maximum synapse weight"),
	TWOPT_UINT("rdn_p", RAND_WT_PROB, "given a λ of 1, if the value is greater than this a neuron will assign a stochastic value to an axon type."),
  TWOPT_GROUP("Benchmark neuron parameters (default run mode)"),

	TWOPT_GROUP("Sim Size Params"),
	TWOPT_UINT("cores", CORES_IN_SIM, "number of cores in simulation"),
	TWOPT_UINT("neurons", NEURONS_IN_CORE, "number of neurons (and axons) in sim"),
    TWOPT_GROUP("Sim tuning"),
  TWOPT_STIME("lh", LH_VAL, "Lookahead setting"),

	TWOPT_FLAG("delta", TW_DELTA, "Use delta encoding for some states"),
	TWOPT_FLAG("simple", BASIC_SOP, "Simple SOPS measurement (simpified neuron model)"),
  TWOPT_STIME("rv", CLOCK_RANDOM_ADJ, "Clock random generator modifier"),
  TWOPT_UINT("rm",CLOCK_RND_MODE, "Clock random mode selector.\n\t\t0 = uniform, 1 = normal, 2 = exponential, 3 = binomal, 4 = simplistic"),
  TWOPT_FLAG("bulk",BULK_MODE,"Is this sim running in bulk mode (called from script?)"),
	TWOPT_GROUP("Debug options"),
	 TWOPT_FLAG("dbg", DEBUG_MODE, "Debug message printing"),
  TWOPT_FLAG("network", SAVE_NEURON_OUTS, "Save neuron output axon IDs on creation"),
    TWOPT_FLAG("svm", SAVE_MEMBRANE_POTS, "Save neuron membrane potential values (enabled by default when running a validation model"),
    TWOPT_FLAG("svs", SAVE_SPIKE_EVTS, "Save neuron spike event times and info"),
    
    TWOPT_FLAG("phval", PHAS_VAL, "Phasic Neuron Validation"),
    TWOPT_FLAG("tonb",TONIC_BURST_VAL, "Tonic bursting Neuron Validation"),
    {TWOPT_END()}

  };
void setSynapseWeight(neuronState *s, tw_lp *lp, int synapseID);
void neuron_event(neuronState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp);
void neuron_reverse(neuronState *, tw_bf *, Msg_Data *, tw_lp *);
void neuron_final(neuronState *s, tw_lp *lp);

void synapse_init(synapseState *s, tw_lp *lp);
void synapse_event(synapseState *s, tw_bf *, Msg_Data *M, tw_lp *lp);
void synapse_reverse(synapseState *, tw_bf *, Msg_Data *M, tw_lp *);
void synapse_final(synapseState *s, tw_lp *lp);

void axon_init(axonState *s, tw_lp *lp);
void axon_event(axonState *s, tw_bf *, Msg_Data *M, tw_lp *lp);
void axon_reverse(axonState *, tw_bf *, Msg_Data *M, tw_lp *);
void axon_final(axonState *s, tw_lp *lp);

/**
 * @brief An assist function that displays the model 
 * configuration before a run.
 * @details With debug mode off, this displays all of the important config
 * info. With debug mode on, this displays the mapping and initial values 
 * of all the axons, neurons, and synapses, along with more detailed
 * information. Only displays on the primary node.
 */
void displayModelSettings();
/**
 * @brief Creates a network of neurons with probabilistic thresholds and remote connections.
 * */
void createProbNeuron(neuronState *s, tw_lp *lp);
void createProbAxon(neuronState *s, tw_lp *lp);

#endif /* defined(__ROSS_TOP__model_main__) */
