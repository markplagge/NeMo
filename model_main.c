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

void createLPs() {
  tw_define_lps(g_tw_nlp, sizeof(Msg_Data), 0);
  for (int i = 0; i < g_tw_nlp; i++) {
  }
}

int main() {
  return 0;
}
