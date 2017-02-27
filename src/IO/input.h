//
// Created by Mark Plagge on 2/21/17.
//

#ifndef SUPERNEMO_INPUT_H
#define SUPERNEMO_INPUT_H

#include "../lib/csv.h"
#include "../globals.h"
#include "../neuro/tn_neuron.h"


int openInputFiles();
int initNetworkFile();
int closeNetworkFile();
int closeSpikeFile();
enum lpTypeVals* neuronMap;


struct CsvNeuron getNeuronData(id_type core, id_type nid);
void parseNetworkFile();
void postParseCleanup();

typedef enum ReadStatus{
    loaded = 0,
    inNeurons = 1,
    myCoreGreater = 2,
    myCoreLower = 3,
    eof = 4
}readStatus;
#endif //SUPERNEMO_INPUT_H
