//
// Created by Mark Plagge on 5/25/16.
//

#include "nemo_main.h"

/** Set for testing IO ops */
#define TESTIO 0

/** \addtogroup Globals
 * @{  */

size_type CORES_IN_SIM = 16;
// size_type AXONS_IN_CORE = NEURONS_IN_CORE;
size_type SIM_SIZE = 1025;
size_type SYNAPSES_IN_CORE = 0;
size_type CORE_SIZE = 0;
size_type LPS_PER_PE = 0;

bool IS_RAND_NETWORK = true;
bool BULK_MODE = false;
bool DEBUG_MODE = false;
bool SAVE_MEMBRANE_POTS = false;
bool SAVE_SPIKE_EVTS = true;
bool SAVE_NETWORK_STRUCTURE = true;
bool PHAS_VAL = false;
bool TONIC_SPK_VAL = false;
bool TONIC_BURST_VAL = false;
bool PHASIC_BURST_VAL = false;
bool VALIDATION = false;
bool MPI_SAVE = false;
bool BINARY_OUTPUT = false;

char *NEURON_FIRE_R_FN = "fire_record";
char *NETWORK_CFG_FN = "nemo_model.nfg1";


//int N_FIRE_BUFF_SIZE = 32;
//int N_FIRE_LINE_SIZE = 512;


//
/**
 * @FILE_OUT - is set to true if NeMo is saving output files
 * @FILE_IN - is set to true if NeMo is reading a file.
 *
 */
bool FILE_OUT = false;
bool FILE_IN = true;

/**
 * outFile - basic output file handler.
 */


//-----------------------Non global testing vars---------//

char *couchAddress = "192.168.2.3";

/** @} */
/**
 * app_opt - Application Options. Manages the options for NeMo's run.
 */
const tw_optdef app_opt[] = {
		TWOPT_GROUP("Input Parameters"),
		//	TWOPT_FLAG("rand_net", IS_RAND_NETWORK, "Generate a random network?
		//Alternatively, you need to specify config files."),
		TWOPT_FLAG("netin", FILE_IN,
				   "Load network information from a file. If set,"
						   "a network file name must be specified.\n If off, a randomly "
						   "generated benchmark model will be used."),
		//TWOPT_CHAR("nfn", NETWORK_CFG_FN, "Input Network File Name"),
		//TWOPT_CHAR("sfn", SPIKE_IN_FN, "Spike input file name"),
		// TWOPT_UINT("tm", testingMode, "Choose a test suite to run. 0=no tests,
		// 1=mapping tests"),
		TWOPT_GROUP("General Network Parameters"),
		// TWOPT_GROUP("Randomized (ID Matrix) Network Parameters"),
		TWOPT_ULONGLONG("cores", CORES_IN_SIM, "number of cores in simulation"),
		// TWOPT_ULONGLONG("neurons", NEURONS_IN_CORE, "number of neurons (and
		// axons) in sim"),
		TWOPT_GROUP("Data Gathering Settings"),
		// TWOPT_FLAG("bulk", BULK_MODE, "Is this sim running in bulk mode?"),
		TWOPT_FLAG("dbg", DEBUG_MODE, "Debug message printing"),
		TWOPT_FLAG("network", SAVE_NETWORK_STRUCTURE, "Save neuron output axon IDs "
				"on creation - Creates a map "
				"of the neural network."),
		TWOPT_FLAG(
				"svm", SAVE_MEMBRANE_POTS,
				"Save neuron membrane potential "
						"values (saves membrane potential per-tick if neuron was active.)"),
		TWOPT_FLAG("svs", SAVE_SPIKE_EVTS,
				   "Save neuron spike event times and info"),

		TWOPT_END()

};

/**
 * model_lps - contains the LP type defs for NeMo
 */
tw_lptype model_lps[] = {
		{

				(init_f) axon_init,    (pre_run_f) NULL,       (event_f) axon_event,
				(revent_f) axon_reverse,     (commit_f) NULL,      (final_f) axon_final,
				(map_f) getPEFromGID, sizeof(axonState)},
		{       (init_f) synapse_init, (pre_run_f) NULL,       (event_f) synapse_event,
				(revent_f) synapse_reverse,  (commit_f) NULL,      (final_f) synapse_final,
				(map_f) getPEFromGID, sizeof(synapseState)},
		{       (init_f) TN_init,      (pre_run_f) NULL, (event_f) TN_forward_event,
				(revent_f) TN_reverse_event, (commit_f) TN_commit, (final_f) TN_final,
				(map_f) getPEFromGID, sizeof(tn_neuron_state)},
		{       0}};

/**
 * @brief      Displays NeMo's initial run size configuration.
 */
void displayModelSettings() {
	if (g_tw_mynode == 0) {
		for (int i = 0; i < 30; i++) {
			printf("*");
		}
		double cores_per_node = CORES_IN_SIM / tw_nnodes();
		char *netMode = FILE_IN ? "file defined" : "random benchmark";
		printf("\n");
		printf("* \t %i Neurons per core (cmake defined), %llu cores in sim.\n",
			   NEURONS_IN_CORE, CORES_IN_SIM);
		printf("* \t %f cores per PE, giving %llu LPs per pe.\n", cores_per_node,
			   g_tw_nlp);
		printf("* \t Neurons have %i axon types (cmake defined)\n",
			   NUM_NEURON_WEIGHTS);
		printf("* \t Network is a %s network.\n", netMode);
		printf("* \t Network Input FileName: %s \n", MODEL_FILE);
		printf("* \t Spike Input FileName %s \n", SPIKE_FILE);
		printf("* \t Neuron stats:\n");
		printf("* \tCalculated sim_size is %llu\n", SIM_SIZE);

		for (int i = 0; i < 30; i++) {
			printf("*");
		}
		printf("\n");
	}
}

/** @brief Does initial tests of Neuron Output subsystem.
 * If subsystem tests are on, then this will "simulate" a series of neuron
 * firing events after
 * initializing file systems.
 *
 * Tests file closing function as well.
 */

void testNeuronOut() {
	SAVE_SPIKE_EVTS = true;
	initOutFiles();

	for (int i = 0; i < 4096; i++) {
		saveNeuronFire(random() + i, 0, 0, 1024);
	}
	closeFiles();
}

/**
 * @brief      Initializes NeMo
 *
 * First, this function checks for potential file IO, and creates file handles
 * for use.
 *
 * Based on the file_in option, the function then sets the neuron, axon, and
 * synapse
 * function pointers to the proper values. Default is the IBM TrueNorth neuron
 * model.
 * If NeMo is reading a file, then we set the size of the sim based on the model
 * config file.
 *
 * The rest of this function manages ROSS initialization and setup. When done,
 *
 */
void init_nemo() {

	VALIDATION = PHAS_VAL || TONIC_BURST_VAL || PHASIC_BURST_VAL;
	FILE_OUT = SAVE_SPIKE_EVTS || SAVE_NETWORK_STRUCTURE || SAVE_MEMBRANE_POTS ||
			   VALIDATION;

	if (FILE_OUT) {
		// Init file output handles
		initOutFiles();
		openOutputFiles("network_def.csv");
		initDataStructures(g_tw_nlp);
		if (g_tw_mynode == 0) {

			printf("Output Files Init.\n");
		}
	}

	if (FILE_IN) {
		// Init File Input Handles
		printf("Network Input Active");
		printf("Filename specified: %s\n", MODEL_FILE);
		//spikeFileName = SPIKE_IN_FN;
		// INPUT Model file init here:
///////////////////////////////////////////////
		initModelInput(CORES_IN_SIM);

// INPUT SPIKE FILE init HERE:
		////////////////////////
		int spkCT = loadSpikesFromFile(SPIKE_FILE);
		printf("Read %i spikes\n", spkCT);
		
	}

	// AXONS_IN_CORE = NEURONS_IN_CORE;
	SYNAPSES_IN_CORE = 1; //(NEURONS_IN_CORE * AXONS_IN_CORE);

	CORE_SIZE = SYNAPSES_IN_CORE + NEURONS_IN_CORE + AXONS_IN_CORE;
	SIM_SIZE = CORE_SIZE * CORES_IN_SIM;

	g_tw_nlp = SIM_SIZE / tw_nnodes();
	g_tw_lookahead = 0.001;
	g_tw_lp_types = model_lps;
	g_tw_lp_typemap = lpTypeMapper;

	/// EVENTS PER PE SETTING
	g_tw_events_per_pe = NEURONS_IN_CORE * AXONS_IN_CORE; // magic number

	LPS_PER_PE = g_tw_nlp / g_tw_npe;
	// Pre-Run Quck Info
}

/**
 * @brief      NeMo Main entry point
 *
 * @param[in]  argc  The argc
 * @param      argv  The argv
 *

 */

int main(int argc, char *argv[]) {
//	char *NETWORK_FILE_NAME = calloc(256, sizeof(char));
//	strcpy(NETWORK_FILE_NAME, "nemo_model.csv");
//	char *SPIKE_FILE_NAME = calloc(256, sizeof(char));
//	strcpy(SPIKE_FILE_NAME, "nemo_spike.csv");
	tw_opt_add(app_opt);
	tw_init(&argc, &argv);

	// call nemo init
	init_nemo();
	if (nonC11 == 1)
		printf("Non C11 compliant compiler detected.\n");

	//    if (testingMode == 1 ) {
	//        unsigned char mapResult = 0;
	//        mapResult = mapResult | mapTests();
	//
	//        if(mapResult & INVALID_AXON){
	//            printf("Creted invalid axon.\n");
	//            }
	//        if (mapResult & INVALID_SYNAPSE){
	//            printf("Created invalid synapse.\n");
	//        }
	//        if (mapResult & INVALID_NEURON){
	//            printf("Created invalid neuron.\n");
	//        }
	//
	//
	//        return mapResult;
	//    }
	// Define LPs:
	tw_define_lps(LPS_PER_PE, sizeof(messageData));
	tw_lp_setup_types();

	if (g_tw_mynode == 0) {
		displayModelSettings();
	}
	tw_run();
	if (SAVE_NETWORK_STRUCTURE) {
		closeOutputFiles();
	}
	if (FILE_OUT) {
		closeFiles();
	}
	tw_end();
}
