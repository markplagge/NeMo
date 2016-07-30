//
// Created by Mark Plagge on 5/25/16.
//

#include "IOStack.h"

FILE * outputFile;
FILE * inputFile;
bool inputFileOpen;

FILE * neuronFireFile;
bool neuronFireFileOpen;
char * neuronFireFileOutput;

char * neuronFireBuffer[N_FIRE_BUFF_SIZE];
int neuronPoolPos = 0;
char * mt;

void flushNeuron(){
    while(neuronPoolPos --){
        fprintf(neuronFireFile, "%s\n", neuronFireBuffer[neuronPoolPos]);
    }
}


void initFiles(){
	if(SAVE_SPIKE_EVTS) {
        for (int i = 0; i < N_FIRE_BUFF_SIZE; i++) {
            neuronFireBuffer[i] = (char *) calloc(sizeof(char), N_FIRE_LINE_SIZE);
        }
        mt = (char *) calloc(sizeof(char), 128);
        mt = sprintf("spike_evt_r-%i.csv", g_tw_mynode);
        neuronFireFile = fopen(mt, "w");
        free(mt);
    }


}
int completedFiles = 0;

void closeFiles(){
    if(g_tw_mynode == 0) {
        flushNeuron();
        FILE *finalout = fopen("neuron_spike_evts.csv", "w");
        fprintf(finalout, "timestamp,neuron_core,neuron_local,destGID\n");

        char *currentFileName = (char *) calloc(sizeof(char), 128);

        FILE * cf;
        for (int i = 0; i < tw_nnodes(); i++) {
            currentFileName = sprintf("spike_evt_r-%i.csv", i);

        }

        fclose(neuronFireFile);
    } else {
        flushNeuron();
        fclose(neuronFireFile);
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
	
}
