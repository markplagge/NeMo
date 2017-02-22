//
// Created by Mark Plagge on 2/21/17.
//

#include "input.h"

static char* networkTempName;
static FILE* networkTempFile;

int openInputFiles(){
    networkFile = fopen(networkFileName,"r");
    if (errno){
        printf("Error opening network def "
                       "file %s, with error code %i.\n",networkFileName, errno);
        return errno;
    }

    networkTempName = calloc(64, sizeof(char));
    networkTempName = sprintf(networkTempName,"NeMoNetworkTemp_rank%l.dat", g_tw_mynode);
    networkTempFile = fopen(networkTempName, "wb");

    if (errno){
        printf("Error saving network temp file. Error code %i  \n", errno);
        return errno;
    }

    return 0;

}

/** Callback function called when libCSV has read an entire field */
void fld_read (void *s, size_t len, void *data){

}

