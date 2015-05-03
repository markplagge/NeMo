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

#include "libs/sqlite3.h"
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
void neuronEventRecord(regid_t core, regid_t local, regid_t fromSynapse, tw_stime timestamp, long postPot);
void synapseEventRecord(regid_t core, regid_t local, tw_stime timestamp, int dest);
void mapRecord( int type, char* typet, int localID, int coreID, int lpid);
#endif /* defined(__ROSS_TOP__stats_collection__) */
