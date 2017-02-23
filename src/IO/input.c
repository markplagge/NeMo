//
// Created by Mark Plagge on 2/21/17.
//

#include "input.h"

static char* networkTempName;
static FILE* networkTempFile;
static FILE* networkTempFiles[];
int openInputFiles(){
    networkFile = fopen(networkFileName,"r");
    if (errno){
        printf("Error opening network def "
                       "file %s, with error code %i.\n",networkFileName, errno);
        return errno;
    }

    int ws = g_tw_npe;

    networkTempFiles = calloc(ws,sizeof(FILE*));

    for (int i = 0; i < ws; i ++) {
        networkTempName = calloc(64, sizeof(char));
        networkTempName = sprintf(networkTempName, "NeMoNetworkTemp_rank%l.dat", g_tw_mynode);
        networkTempFile = fopen(networkTempName, "wb");
        networkTempFiles[i] = networkTempFile;
    }
    if (errno){
        printf("Error saving network temp file. Error code %i  \n", errno);
        return errno;
    }

    return 0;

}

/** Callback function called when libCSV has read an entire field */
void fld_read (void *s, size_t len, void *data){
    //When a field is read, the data in string format is given here.
    //assemble a complete data structure based on the type (use state machine st 1)
    //cast the values before storing in the struct.
}
void line_read(void *s, size_t len, void *data) {
    //once a line has been read in, look at the global struct where data is stored.
    //convert the csv struct into a neuron state struct.
    //next, save the neuron state struct into a binary file (append)
    //pick the binary file based on which rank the NS core should run on.

}
void parseNetworkFile(){
    //Only run on MPI first rank
    //OR
    //Run in parallel, ignoring lines that have a different core than what this
    //process' rank would be running.
    //read CSV file line by line.
    //Parse the simulation parameters, and store them
    //Once in neuron defs, start using CSV_PARSE with the fld_read callback until done.
}

void readNeurons(){

}