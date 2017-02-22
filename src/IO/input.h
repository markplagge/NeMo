//
// Created by Mark Plagge on 2/21/17.
//

#ifndef SUPERNEMO_INPUT_H
#define SUPERNEMO_INPUT_H

#include "../lib/csv.h"
#include "../globals.h"
#include "../neuro/tn_neuron.h"
char * networkFileName;
char * spikeFileName;

FILE * networkFile;
FILE * sortedNetFile;

int openInputFiles();

int closeNetworkFile();
int closeSpikeFile();




#endif //SUPERNEMO_INPUT_H
