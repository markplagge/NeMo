//
// Created by Mark Plagge on 9/5/18.
//

#include "IOStack.h"
#include "../neuro/tn_neuron_struct.h"

void loadNeuronFromJSON(id_type neuronCore, id_type neuronLocal, tn_neuron_state *n){
  init_neuron_state(neuronCore,neuronLocal, n);
  return;
  tn_neuron_state *newState = get_neuron_state(neuronCore, neuronLocal);
  free(n);
  *n = *newState;
  n->myCoreID = newState->myCoreID;

}

void initJSON(char *jsonFilename){

  load_and_init_json_model(jsonFilename);
}

/**
 * Cleans up the open JSON file and data structures.
 * \todo Need to implement this to save memory!
 */
void closeJSON(){
  //clean_up();

}