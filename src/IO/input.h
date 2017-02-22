//
// Created by Mark Plagge on 2/21/17.
//

#ifndef SUPERNEMO_INPUT_H
#define SUPERNEMO_INPUT_H

#include "../lib/csv.h"

char * networkFileName;
char * spikeFileName;


int openInputFiles();
int closeNetworkFile();
int closeSpikeFile();




#endif //SUPERNEMO_INPUT_H
