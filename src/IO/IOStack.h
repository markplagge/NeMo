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
void initModelInput(char *filename, int maxNeurons);

#endif //NEMO_IOSTACK_H
