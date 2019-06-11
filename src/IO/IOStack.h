//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_IOSTACK_H
#define NEMO_IOSTACK_H
#include <stdio.h>
#include <stdlib.h>
#include "../globals.h"
#include "../nemo_config.h"



void initOutFiles();
void closeFiles();

void saveEvent(tw_stime timestamp, char sourceType, id_type core, id_type local,
               id_type destCore, id_type destLocal);



void saveNeuronFire(tw_stime timestamp, id_type core, id_type local, tw_lpid destGID);


/** @defgroup PRAS_DATA
 * Pras' data file handlers.
 * These functions manage the input files for Pras' data.
 * For ease of working, they simply open the files and read them,
 * returning the relevent information or setting the values as needed
 */
typedef struct SpikeData{
    int source;
    int dest;
    double dest_time;
}spikeData;
void prGetWeights(int *weights, long long coreID, long long neuronID);

/**
 * Loads ALL spike data from the file  and returns it as a spike array.
 * @return initialized! array of spikeData structs
 */
spikeData * prLoadSpikeData();

#endif //NEMO_IOSTACK_H
