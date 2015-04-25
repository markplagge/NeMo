//The C main file for a ROSS model
//This file includes:
// - command line argument setup
// - a main function

//includes
#include "ross.h"
#include "model.h"

//add your command line opts
const tw_optdef model_opts[] = {
	TWOPT_GROUP("ROSS Model"),
	TWOPT_UINT("setting_1", setting_1, "first setting for this model"),
	TWOPT_END(),
};


//for doxygen
#define model_main main

int model_main (int argc, char* argv[]) {
	int i;
	int num_lps_per_pe;

	tw_opt_add(model_opts);
	tw_init(&argc, &argv);

	//Do some error checking?
	//Print out some settings?

	//Custom Mapping
	/*
	g_tw_mapping = CUSTOM;
	g_tw_custom_initial_mapping = &model_custom_mapping;
	g_tw_custom_lp_global_to_local_map = &model_mapping_to_lp;
	*/

	//Useful ROSS variables and functions
	// tw_nnodes() : number of nodes/processors defined
	// g_tw_mynode : my node/processor id (mpi rank)

	//Useful ROSS variables (set from command line)
	// g_tw_events_per_pe
	// g_tw_lookahead
	// g_tw_nlp
	// g_tw_nkp
	// g_tw_synchronization_protocol

	//assume 1 lp per node
	num_lps_per_pe = 1;

	//set up LPs within ROSS
	tw_define_lps(num_lps_per_pe, sizeof(message), 0);
	// g_tw_nlp gets set by tw_define_lps

	for (i = 0; i < g_tw_nlp; i++) {
		tw_lp_settype(i, &model_lps[0]);
	}

	// Do some file I/O here? on a per-node (not per-LP) basis

	tw_run();

	tw_end();

	return 0;
}
