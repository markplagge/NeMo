//
// Created by Mark Plagge on 5/25/16.
//

#include <stdio.h>
#include "nemo_main.h"

/** \addtogroup Globals 
 * @{  */

size_type CORES_IN_SIM = 1024; 
size_type NEURONS_IN_CORE = 512;
size_type AXONS_IN_CORE = 512;
size_type SIM_SIZE = 1025;
size_type SYNAPSES_IN_CORE = 0;
size_type CORE_SIZE = 0;

bool IS_RAND_NETWORK = true;
bool BULK_MODE = false;
bool DEBUG_MODE = false;
bool SAVE_MEMBRANE_POTS  = false;
bool SAVE_SPIKE_EVTS  = false;
bool SAVE_NEURON_OUTS = false;
bool PHAS_VAL = false;
bool TONIC_SPK_VAL = false;
bool TONIC_BURST_VAL = false;
bool PHASIC_BURST_VAL = false;
bool VALIDATION = false;



/**
 * @FILE_OUT - is set to true if NeMo is saving output files
 * @FILE_IN - is set to true if NeMo is reading a file.
 * 
 */
bool FILE_OUT = false;
bool FILE_IN = false;

/**
 * outFile - basic output file handler.
 */
FILE *outFile;

/** @} */
/**
 * app_opt - Application Options. Manages the options for NeMo's run.
 */
const tw_optdef app_opt[] = {
	TWOPT_FLAG("rand_net", IS_RAND_NETWORK, "Generate a random network? Alternatively, you need to specify config files."),
	
	TWOPT_GROUP("Randomized (ID Matrix) Network Parameters"),
		TWOPT_ULONGLONG("cores", CORES_IN_SIM, "number of cores in simulation"),
    	TWOPT_ULONGLONG("neurons", NEURONS_IN_CORE, "number of neurons (and axons) in sim"),
    TWOPT_GROUP("Data Gathering Settings"),
    	TWOPT_FLAG("bulk", BULK_MODE, "Is this sim running in bulk mode?"),
    	TWOPT_FLAG("dbg", DEBUG_MODE, "Debug message printing"),
    	TWOPT_FLAG("network", SAVE_NEURON_OUTS, "Save neuron output axon IDs on creation"),
    	TWOPT_FLAG("svm", SAVE_MEMBRANE_POTS, "Save neuron membrane potential values (enabled by default when running a validation model"),
    	TWOPT_FLAG("svs", SAVE_SPIKE_EVTS, "Save neuron spike event times and info"),
    TWOPT_GROUP("Integrated Bio Model Testing"),
    	TWOPT_FLAG("phval", PHAS_VAL, "Phasic Neuron Validation"),
    	TWOPT_FLAG("tonb",TONIC_BURST_VAL, "Tonic bursting Neuron Validation"),
    	TWOPT_FLAG("phb", PHASIC_BURST_VAL, "Phasic Bursting Neuron Validation"),
    {TWOPT_END()}

};


/**
 * model_lps - contains the LP type defs for NeMo
 */
tw_lptype model_lps[] = {
	{

        (init_f)axon_init,
        (pre_run_f)NULL,
        (event_f)axon_event,
        (revent_f)axon_reverse,
        (final_f)axon_final,
        (map_f)getPEFromGID,
        sizeof(axonState) },
    {
        (init_f)synapse_init, 
        (pre_run_f)NULL,
        (event_f)synapse_event,
        (revent_f)synapse_reverse,
        (final_f)NULL,
        (map_f)getPEFromGID, 
        sizeof(synapseState)
    },
    {
        (init_f)neuron_init,
        (pre_run_f)NULL,
        (event_f)neuron_event,
        (revent_f)neuron_reverse,
        (final_f)NULL,
        (map_f)getPEFromGID, 
        sizeof(neuronState)
    }
    ,
        { 0 } };

/**
 * @brief      Initializes NeMo
 * 
 * First, this function checks for potential file IO, and creates file handles for use.
 * 
 * Based on the file_in option, the function then sets the neuron, axon, and synapse 
 * function pointers to the proper values. Default is the IBM TrueNorth neuron model.
 * If NeMo is reading a file, then we set the size of the sim based on the model config file.
 * 
 * The rest of this function manages ROSS initialization and setup. When done,
 * 
 */
void init_nemo(){


	VALIDATION = PHAS_VAL || TONIC_BURST_VAL || PHASIC_BURST_VAL;
	FILE_OUT = SAVE_SPIKE_EVTS || SAVE_NEURON_OUTS || 
				SAVE_MEMBRANE_POTS || VALIDATION;

	FILE_IN = !IS_RAND_NETWORK;


	if (FILE_OUT){
		//Init file output handles
	}

	if (FILE_IN){
		//Init File Input Handles
		//reconfigure cores_in_sim and neurons_in_sim based on loaded file.
		//override default LP function pointers
		
	}

	
	AXONS_IN_CORE = NEURONS_IN_CORE;
	SYNAPSES_IN_CORE = (NEURONS_IN_CORE * AXONS_IN_CORE);
	CORE_SIZE = SYNAPSES_IN_CORE + NEURONS_IN_CORE + AXONS_IN_CORE;
	SIM_SIZE = CORE_SIZE * CORES_IN_SIM;

	g_tw_nlp = SIM_SIZE / tw_nnodes();
	g_tw_lookahead = 0.001;



	///EVENTS PER PE SETTING
	g_tw_events_per_pe = 65536; //magic number 

}

/**
 * @brief      NeMo Main entry point
 *
 * @param[in]  argc  The argc
 * @param      argv  The argv
 *
 
 */
int main(int argc, char*argv[]) {
	tw_opt_add(app_opt);
	tw_init(&argc, &argv);


}
