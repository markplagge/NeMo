//
// Created by Mark Plagge on 5/25/16.
//

#include "IOStack.h"
#include <stdio.h>
#include <string.h>


FILE * outputFile;
FILE * inputFile;

FILE * neuronFireFile;

char * neuronFireBuffer[N_FIRE_BUFF_SIZE];
int neuronPoolPos = 0;

void initFiles(){
	
	for (int i = 0; i < N_FIRE_BUFF_SIZE; i ++){
		neuronFireBuffer[i] = (char *) calloc(sizeof(char), N_FIRE_LINE_SIZE);
	}
	
	if (SAVE_NEURON_OUTS  ) {
		
	}
    
}
void closeFiles(){
    
}

void flushNeuron(){
	while(neuronPoolPos --){
		fprintf(neuronFireFile, "%s\n", neuronFireBuffer[neuronPoolPos]);
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
