//
// Created by Mark Plagge on 5/25/16.
//

#include "IOStack.h"


FILE * inputFile;
bool inputFileOpen;









void flushNeuron(){
    while(--neuronPoolPos > -1){
        fprintf(neuronFireFile, "%s\n", neuronFireBuffer[neuronPoolPos]);
    }
    neuronPoolPos = 0;
}

char * setNetworkFileName(){
    mt = (char *) calloc(128, sizeof(char));
    sprintf(mt, "spike_evt_r-%i.csv", g_tw_mynode);
    printf("---- FN: %s \n", mt);
}
void initFiles(){
	if(SAVE_SPIKE_EVTS) {
        for (int i = 0; i < N_FIRE_BUFF_SIZE; i++) {
            neuronFireBuffer[i] = (neuronFireStruct *) calloc(sizeof(neuronFireStruct), 1);
        }
        setNetworkFileName();
        neuronFireFile = fopen(mt, "wb");
//
//        MPI_File_open(MPI_COMM_WORLD,mpiFileName,
//                      MPI_MODE_CREATE|MPI_MODE_WRONLY,MPI_INFO_NULL,neuronFireFileMPI);
//
//        MPI_File_set_atomicity(neuronFireFileMPI,1);

    }


}

void closeFiles(){
    flushNeuron();
    fclose(neuronFireFile);
//    MPI_File_close(neuronFireFileMPI);
    MPI_Barrier(MPI_COMM_WORLD); // wait for everyone to catch up.
    if(g_tw_mynode == 0) {

        FILE *finalout = fopen("neuron_spike_evts.csv", "w");
        fprintf(finalout, "timestamp,neuron_core,neuron_local,destGID\n");

    }


}


void saveEvent(tw_stime timestamp, char sourceType, id_type core, id_type local,
               id_type destCore, id_type destLocal){
	

	
}

void saveNeuronFire(tw_stime timestamp, id_type core, id_type local, tw_lpid destGID){
	if (neuronPoolPos > N_FIRE_BUFF_SIZE){
		flushNeuron();
	}
	
	sprintf(neuronFireBuffer[neuronPoolPos],"%d,%ld,%ld,%ld",
	        timestamp,core,local,destGID);

    ++ neuronPoolPos;
}
