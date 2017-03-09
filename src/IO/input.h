//
// Created by Mark Plagge on 2/21/17.
//

#ifndef SUPERNEMO_INPUT_H
#define SUPERNEMO_INPUT_H

#include "../lib/csv.h"
#include "../globals.h"
#include "../neuro/tn_neuron.h"
#include <sys/queue.h>

enum neuronTypeVals* neuronMap;
char * neuronFireFileName;
char * networkFileName;
char * spikeFileName;
typedef enum ReadStatus{
	loaded = 0,
	inNeurons = 1,
	myCoreGreater = 2,
	myCoreLower = 3,
	eof = 4
}readStatus;


int openInputFiles();
int initNetworkFile();
void parseNetworkFile();
void postParseCleanup();


double  * getNextSpikeFromFile();
int queueSpikesFromAxon(id_type coreID, id_type localID);

int closeNetworkFile();
int closeSpikeFile();

struct CsvNeuron getNeuronData(id_type core, id_type nid);




#endif //SUPERNEMO_INPUT_H
