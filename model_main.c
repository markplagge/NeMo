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
                             (map_f)mapping,
                             sizeof(neuronState)},
                         {

                             (init_f)synapse_init,
                             (pre_run_f)pre_run,
                             (event_f)synapse_event,
                             (revent_f)synapse_reverse,
                             (final_f)synapse_final,
                             (map_f)mapping,
                             sizeof(synapseState)},

                         {(init_f)axon_init,
                          (pre_run_f)pre_run,
                          (event_f)axon_event,
                          (revent_f)axon_reverse,
                          (final_f)axon_final,
                          (map_f)mapping,
                          sizeof(axonState)},

                         {0}

};

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
	int noff = CORE_SIZE - NEURONS_IN_CORE;
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
        printf("Created %i axons, %i synapses,  %i neurons", axons, synapses, neurons);
}

int main(int argc, char *argv[]) {
  //set up core sizes.
  SYNAPSES_IN_CORE = NEURONS_IN_CORE * AXONS_IN_CORE;
  CORE_SIZE = SYNAPSES_IN_CORE + NEURONS_IN_CORE + AXONS_IN_CORE;
  tw_opt_add(app_opt);

    tw_init(&argc, &argv);
  /** g_tw_nlp set here to CORE_SIZE.
   * @todo check accuracy of this
   * */


  g_tw_events_per_pe = CORE_SIZE * eventAlloc;
  ///@todo enable custom mapping with these smaller LPs.

  g_tw_mapping = ROUND_ROBIN;
  g_tw_lp_types = model_lps;

  ///@todo do we need a custom lookahedad parameter?
  createLPs();
  tw_run();
   tw_end();
  return 0;
}




//neuron functions
void neuron_init(neuronState *s, tw_lp *lp)
{

}

void setSynapseWeight(neuronState *s, tw_lp *lp, int synapseID)
{

}


void neuron_event(neuronState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp)
{

}



void neuron_reverse(neuronState *s, tw_bf *CV, Msg_Data *MCV, tw_lp *lp)
{

}


void neuron_final(neuronState *s, tw_lp *lp)
{

}

//synapse function
void synapse_init(synapseState *s, tw_lp *lp)
{

}


void synapse_event(synapseState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp)
{

}


void synapse_reverse(synapseState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp)
{

}


void synapse_final(synapseState *s, tw_lp *lp)
{

}

// Axon function.
void axon_init(axonState *s, tw_lp *lp)
{

}

void axon_event(axonState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp)
{

}


void axon_reverse(axonState *s, tw_bf *CV, Msg_Data *M, tw_lp *lp)
{

}


void axon_final(axonState *s, tw_lp *lp)
{

}





void mapping(tw_lp gid)
{

}


void pre_run()
{

}
