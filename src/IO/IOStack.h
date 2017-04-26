//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_IOSTACK_H
#define NEMO_IOSTACK_H
#include <stdio.h>
#include <stdlib.h>
#include "input.h"
#include "output.h"
#include "../globals.h"
#include "../lib/simclist.h"
#include "../nemo_config.h"



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
#endif //NEMO_IOSTACK_H
