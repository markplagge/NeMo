//
//  stats_collection.c
//  TNT_BENCHMARK
//
//  Created by Mark Plagge on 5/2/15.
//
//

#include "stats_collection.h"
static int callback(void *data, int argc, char **argv, char **azColName){
	int i;
	fprintf(stderr, "%s: ", (const char*)data);
	for(i=0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	printf("SQL Created \n");
	return 0;
}
const char* data = "Callback function called";
sqlite3 *db;
void initDB(){

	char *zErrMsg = 0;
	int rc;
	char* synSQL = "CREATE TABLE SynapseEvents"
	" (eventID INTEGER NOT NULL,"
	" coreID INTEGER,"
	" localID INTEGER,"
	" eventTime DOUBLE,"
	" sent INTEGER,"
	" PRIMARY KEY (eventID))";

	char* neuSQL = ""
	" CREATE TABLE neuronEvent"
	" (eventID INTEGER NOT NULL,"
	" coreID INTEGER,"
	" localID INTEGER,"
	" eventTime DOUBLE,"
	" receivedSynapse INTEGER,"
	" postFirePotential INTEGER,"
	" PRIMARY KEY (eventID))";
	rc = sqlite3_open("./livestats.db", &db);

	char* mapSQL = "CREATE TABLE mappings"
	" (ID INTEGER NOT NULL,"
	" type TEXT,"
	" typeImp TEXT,"
	" core INTEGER,"
	" local INTEGER,"
	" LPID INTEGER,"
	"PRIMARY KEY (ID))";

	rc = sqlite3_exec(db, synSQL, callback, (void*)data, &zErrMsg);
	rc = sqlite3_exec(db, neuSQL, callback, (void*)data, &zErrMsg);
		rc = sqlite3_exec(db, mapSQL, callback, (void*)data, &zErrMsg);
	printf("Created database. \n");
	sqlite3_close(db);
}

void mapRecord( int type, char* typet, int localID, int coreID, int lpid){
	static int val = 0;
	char* tpt = type == 0? "Neuron":"Synapse";
	char* sql = sqlite3_mprintf("INSERT INTO mappings (ID, type, typeImp, core, local, LPID) VALUES (%i, %Q, %Q, %i, %i, %i);", val, tpt, typet, coreID, localID, lpid);
	char* zErrMsg = 0;
	int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
	val ++;
}
void neuronEventRecord(regid_t core, regid_t local, regid_t
	fromSynapse, tw_stime timestamp, long postPot){
	static int row = 0;
	char *zErrMsg=0;


	char* sql = sqlite3_mprintf("INSERT INTO neuronEvent (rowid, eventID, coreID, localID, eventTime, receivedSynapse, postFirePotential) VALUES (%i, %i, %i, %i, %f, %i, %i); ",row,row,core,local,timestamp,fromSynapse,postPot);


	int rc;
	rc = sqlite3_open("./livestats.db", &db);

	 rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);

}
void startRecord(){
	sqlite3_open("./livestats.db", &db);
}
void endRecord(){
	sqlite3_close(db);
}
void synapseEventRecord(regid_t core, regid_t local, tw_stime timestamp, int dest){
	static int row = 0;
	char *zErrMsg=0;
	char *sql = sqlite3_mprintf("insert into SynapseEvents ( \"localID\", \"coreID\", \"eventTime\", \"eventID\", \"sent\") values (%i,%i,%f,%i,%i);", local, core, timestamp, row, dest);
	int rc;


	rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
	row ++;


}