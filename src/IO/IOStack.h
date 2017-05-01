//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_IOSTACK_H
#define NEMO_IOSTACK_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "output.h"
#include "../globals.h"
#include "../lib/simclist.h"
#include "../nemo_config.h"
//#include "../lib/lua.h"
//#include "../lib/lualib.h"
//#include "../lib/lauxlib.h"


#define SAVE_ALL_NEURON_PARAMS 1


void initOutFiles();
void closeFiles();

//void saveEvent(tw_stime timestamp, char sourceType, id_type core, id_type local,
//               id_type destCore, id_type destLocal);



void saveNeuronFire(tw_stime timestamp, id_type core, id_type local, tw_lpid destGID);
void openOutputFiles(char * outputFileName);
void initDataStructures(int simSize);

void closeOutputFiles();
void saveIndNeuron(void *n);

int getNeuronParameters(double *paramArray, int core, int local);
/**
 * initNeuronFromCSV - configures a neuron using the loaded CSV data.
 * Neurons must be passed in from the initialization function, already allocated.
 * If the neuron is not found in the CSV file, the neuron state is not changed
 * and the function returns -1.
 * @param core The core of the neuron.
 * @param local The local (corewise) ID of the neuron.
 * @param neuron The neuron state
 * @param neurohnConstructor a pointer to the neuron constructor function. If null, the state is directly loaded
 * @param neuronType the neuron type. TrueNorth neurons are 'T'.
 */
int initNeuronFromCSV(int core, int local, void * neuron, void * neuronConstructor, char neuronType);
/**
 * Prepares the model file for loadiing. Loads the file into memory and stores it in a list.
 * @param filename the model filename, must be a NeMo CSV
 * @param maxNeurons The maximum number of neurons in the model file. Is an upper bound on the list size. If set to -1,
 * will be estimated from the number of lines in the file.
 */
void initModelInput(char *filename, int maxNeurons);

#endif //NEMO_IOSTACK_H
