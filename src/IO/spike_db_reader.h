//
// Created by Mark Plagge on 11/28/17.
//

#ifndef SUPERNEMO_SPIKE_DB_READER_H
#define SUPERNEMO_SPIKE_DB_READER_H
#include "IOStack.h"
#include "../../sqlite/sqlite3.h"
//#include <sqlite3.h>
//#include <sqlite3ext.h>
#include "../globals.h"

#endif //SUPERNEMO_SPIKE_DB_READER_H
//SQLITE FUNCTIONS:

int getSpikeMessageDataFromAxon(int *M, id_type coreID, id_type axonID);
int getSpikeMessageDataFromSynapse(messageData * M, id_type coreID, id_type synapseID);

int checkIfSpikesExistForAxon(id_type coreID, id_type axonID);
int loadOrSaveDb(sqlite3 *pInMemory, const char *zFilename, int isSave) ;

int connectToDB(char * filename);
int closeDB(char* filename);

int testImport();