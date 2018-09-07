//
// Created by Mark Plagge on 9/3/18.
//


#include "../include/tn_parser.hh"
#include "../include/model_reader_wrapper.h"


TN_Main c_model;
TN_Main create_tn_data(char *filename){
  return create_tn_data(string(filename));
}


extern "C" {

#include "../include/model_reader_wrapper.h"
tn_neuron_state *get_neuron_state(unsigned long my_core, unsigned long neuron_id) {
  tn_neuron_state * s = c_model.generate_neuron_from_id(my_core,neuron_id).getTn();
  return s;
}

void init_neuron_state(unsigned long my_core, unsigned long neuron_id, tn_neuron_state *n){
  c_model.populate_neuron_from_id(my_core, neuron_id,n);
}

tn_neuron_state *get_neuron_state_array(int my_core) {
  return c_model.generate_neurons_in_core_struct(my_core);
}

void create_neuron_model(char *filename) {
  printf("\n\n\n -------- Loading neuron model file %s \n", filename);
  c_model = create_tn_data(filename);
}

int serial_load_json(char *json_filename) {
  int result = 0;

}

void clean_up(TN_Main* m){
  delete(m);
}



}
