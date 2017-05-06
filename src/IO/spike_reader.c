//
// Created by Mark Plagge on 4/24/17.
//

#include "spike_reader.h"
char * lineBuff;


int seeker(const void *element, const void *key){
    const spikeElem *spk = (spikeElem *) element;
    const spikeElem *comp = (spikeElem *) key;

    if (spk->destCore == comp->destCore && spk->destAxon == comp->destAxon){
        return 1;
    }
    return 0;
}



int openSpikeFile(char * filename){
    errno = 0;
    spikeFile = fopen(filename,"rb");
    lineBuff = calloc(128, sizeof(char));
    return errno;
}
int closeSpikeFile(){
    return fclose(spikeFile);
    free(lineBuff);
}


int readSpike(spikeElem * spike){
    long axon,core, time = 0;
    char * r = fgets(lineBuff, 128, spikeFile);
    if (r != NULL){
        //int ctr = readLine(r,axon,core,time);
        int ctr = 0;

        ctr = sscanf(r,"%li,%li,%li", &time,&core,&axon);
        if (ctr > 2) {
            spike->scheduledTime = time;
            spike->destCore = core;
            spike->destAxon = axon;

        }

        return ctr;
    }
    return -1;
}

/**
 * Loads spikes from a specified file. Returns the number of elements loaded.
 * @param filename the filename to load
 * @return the number of elements added to the spike list.
 */
int loadSpikesFromFile(char * filename){
    int v = openSpikeFile("/Users/mplagge/development/NeMo/cmake-build-debug/bin/nemo_spike.csv");
    int count = -1;
    if (v != 0){
        printf("Error opening file %s, errorcode %i \n",filename, v);
        closeSpikeFile();
        return count;

    }

    list_init(&spikeList);

    spikeElem * spike = calloc(1, sizeof(spikeElem)); //malloc(sizeof(spikeElem));

    list_attributes_seeker(&spikeList, seeker);

    //list_attributes_copy(&spikeList, spmtr, 1);

    spikeElem * s2 = malloc(sizeof(spikeElem));
    s2->scheduledTime = -1;
    s2->destAxon = -1;
    s2->destCore = -1;
    list_append(&spikeList, s2);

    while(readSpike(spike) > 2){
        list_append(&spikeList, spike);
        spike = calloc(1, sizeof(spikeElem));
    }
    //free(spike);

    count = list_size(&spikeList);

    closeSpikeFile();
    return count;

}

spikeElem * getSpike(long destCore, long destAxon){
    spikeElem * spike = NULL;
    spikeElem spikeSr = {.destAxon=destAxon,
            .destCore=destCore, .scheduledTime=0};

    spike = list_seek(&spikeList, &spikeSr);

    if (spike != NULL){
        int pos = list_locate(&spikeList, spike);
        if ( pos > 0){
            spike = list_extract_at(&spikeList, pos);
        }
    }


    return spike;
}