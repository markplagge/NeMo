//
// Created by Mark Plagge on 4/24/17.
//

#ifndef SUPERNEMO_SPIKE_READER_H
#define SUPERNEMO_SPIKE_READER_H

#include "../lib/simclist.h"
#include <stdio.h>
#include <stdlib.h>
#include "../globals.h"
list_t spikeList;
FILE * spikeFile;


        int openSpikeFile(char * filename);
        int closeSpikeFile();

        int loadSpikesFromFile(char * filename);

        spikeElem * getSpike(long destCore, long destAxon);

        int getSpikeCount();

        void testSpikes();


#endif //SUPERNEMO_SPIKE_READER_H
