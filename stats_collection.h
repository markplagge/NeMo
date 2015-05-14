//
//  stats_collection.h
//  ROSS_TOP
//
//  Created by Mark Plagge on 5/2/15.
//
//



#ifndef __ROSS_TOP__stats_collection__
#define __ROSS_TOP__stats_collection__
#include <stdio.h>
#include "assist.h"
#include "models/neuron_model.h"
	//#include "libs/sqlite3.h"
#include "libs/sqlite3.h"
#include <unistd.h>
#include <sys/wait.h>
/* tables */




/* CREATE Synapse Event */
/* INSERT INTO SynapseEvents (rowid, eventID, coreID, localID, eventTime) VALUES (1, 1, 123, 141, 141.123); */
/* CREATE neuronEvent */
/* INSERT INTO neuronEvent (rowid, eventID, coreID, localID, eventTime, receivedSynapse, postFirePotential) VALUES (1234567890, 1234567890, 1, 123, 4560, 456, 4); */



void initDB();

void writeDataWithName(char* filename);
void writeData();
void startRecord();
void endRecord();
void neuronEventRecord(tw_lpid neuronGlobal, regid_t core, regid_t local, regid_t fromSynapse,
                       tw_stime timestamp, long postPot, char *send);
void synapseEventRecord(tw_lpid gid, regid_t core, regid_t local, tw_stime timestamp, tw_lpid dest);
void mapRecord( int type, char* typet, int localID, int coreID, int lpid, tw_lpid gid);
#endif /* defined(__ROSS_TOP__stats_collection__) */
void recordNeuron(neuronState *n);
void recordOutOfBounds(char * type, unsigned long DlocalID, unsigned long DcoreID, unsigned long long Dglobal, unsigned long sCore, unsigned long sLocal, unsigned long long GID);
void recordError(char * type, char* structName, tw_lpid sourceGID,tw_stime time);
void finalClose();
