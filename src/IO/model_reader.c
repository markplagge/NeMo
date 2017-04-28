//
// Created by Mark Plagge on 4/28/17.
//

#include "IOStack.h"
#include <search.h>
#include "../neuro/tn_neuron.h"



void initModelInput(char *filename, int maxNeurons) {
    hcreate(maxNeurons);
    FILE * mdlF = fopen(filename, "rb");
    if (mdlF != NULL){
        char * itm = calloc(2048, sizeof(char));
    }
    else{

        tw_error(TW_LOC, "Model_Read","Error opening %s \n", filename);
    }
}


/**
 * Finds a neuron in the model config file. fills paramArray with the parameters from a file if
 * the neuron is in the config file. Returns -1 if neuron is not found, otherwise returns the number
 * of parameters found.
 * @param paramArray An initialized array of values, will be filled by the function.
 * @param core The neuron's core
 * @param local The neuron's local ID.
 * @return
 */
int getNeuronParameters(double *paramArray, int core, int local){

}