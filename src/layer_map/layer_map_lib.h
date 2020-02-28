#ifndef LAYER_MAP_LIBRARY_H
#define LAYER_MAP_LIBRARY_H
#include <stdlib.h>

#include <ross.h>
#include "../nemo_main.h"

void setup_grid(int showMapping);

/**@} */

void displayConfig();

tw_lpid get_neuron_dest_in_layer(id_type sourceCore, tw_lpid neuronGID);
void configureNeuronInLayer(tn_neuron_state *s, tw_lp *lp);

#endif