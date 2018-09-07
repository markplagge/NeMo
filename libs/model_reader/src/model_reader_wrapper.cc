//
// Created by Mark Plagge on 9/3/18.
//
#include "../include/model_reader_wrapper.h"
tn_neuron_state *get_neuron_state(unsigned long my_core, unsigned long neuron_id);
tn_neuron_state *get_neuron_state_array(unsigned long my_core, unsigned long neuron_id, int num_neurons);
int serial_load_json(char * json_filename);
