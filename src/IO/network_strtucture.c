//
// Created by Mark Plagge on 2/2/18.
//

#include "IOStack.h"
#include "../../sqlite/sqlite3.h"
#include "../lib/rqueue.h"

rqueue_t *qdat;

sqlite3 *networkDB;
FILE *networkFile;

FILE *spikeFile;
char * netfn = "network.csv";
char * spikefn = "spike.csv";
char *dbfn = "network.sqlite";

int initDB(){
  sqlite3_stmt *res;
//  int rc = sqlite3_prepare_v2(networkDB,)
  return 1;
}

int initFile(){

}


int writeNeuronConnections(neruoCon *neuronData){
  static int dbStat = 0;
  if (dbStat ==  0){
    dbStat = initDB();
    if (dbStat){
      tw_error(TW_LOC, "Database init error code %i \n", dbStat);
    }
  }
  int result = 0;


  return result;
}
int writeNeuronSpike(neuroSpike *spikeInfo){
  int result = 0;


  return result;
}
