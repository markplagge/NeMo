//
//  stats_collection.c
//  TNT_BENCHMARK
//
//  Created by Mark Plagge on 5/2/15.
//
//

#include "stats_collection.h"
static int callback(void *data, int argc, char **argv, char **azColName){
	printf("Debug Database Call \n");
	int i;
	printf("%s: ", (const char*)data);
	for(i=0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	printf("SQL Created \n");
	return 0;
}

const char* data = "Callback function called";
sqlite3 *db;
char* path;
void initDB(){
	path = sqlite3_mprintf("./tnt_bench_stat-%i.db", g_tw_mynode);
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
	" (eventID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
	" coreID INTEGER,"
	" localID INTEGER,"
	" eventTime DOUBLE,"
	" receivedSynapse INTEGER,"
	" postFirePotential INTEGER,"
	" eventType TEXT"
	" )";
	rc = sqlite3_open(path, &db);

	char* mapSQL = "CREATE TABLE mappings"
	" (ID INTEGER NOT NULL,"
	" type TEXT,"
	" typeImp TEXT,"
	" core INTEGER,"
	" local INTEGER,"
	" LPID INTEGER,"
	" gID Integer,"
	"PRIMARY KEY (ID))";
	char* nmapSQL = "CREATE TABLE \"neurons\" ("
	" \"neuronID\" integer NOT NULL,"
	" \"dendCore\" integer,"
	" "
	" \"dendGID\" INTEGER,"
	" \"dendLocal\" integer,"
	" \"thresh\" integer,"
	" \"other\" integer,"
	" \"ID\" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT\n"
	") ";

	rc = sqlite3_exec(db, synSQL, callback, (void*)data, &zErrMsg);
	rc = sqlite3_exec(db, neuSQL, callback, (void*)data, &zErrMsg);
		rc = sqlite3_exec(db, mapSQL, callback, (void*)data, &zErrMsg);
	rc = sqlite3_exec(db, nmapSQL, callback, (void*)data, &zErrMsg);

	printf("Created database. \n");
	sqlite3_close(db);
}

void mapRecord( int type, char* typet, int localID, int coreID, int lpid, tw_lpid gid){
	static int val = 0;
	char* tpt = type == 0? "Neuron":"Synapse";
	char* sql = sqlite3_mprintf("INSERT INTO mappings (ID, type, typeImp, core, local, LPID, gID) VALUES (%i, %Q, %Q, %i, %i, %i, %llu);", val, tpt, typet, coreID, localID, lpid, gid);
	char* zErrMsg = 0;
	int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
	val ++;
}
void neuronEventRecord(regid_t core, regid_t local, regid_t fromSynapse, tw_stime timestamp, long postPot, char *send){
	static int row = 0;
	char *zErrMsg=0;


	char* sql = sqlite3_mprintf("INSERT INTO neuronEvent ( coreID, localID, eventTime, receivedSynapse, postFirePotential, eventType) VALUES (%i, %i, %f, %i, %i, %Q); ",core,local,timestamp,fromSynapse,postPot, send);


	int rc;


	 rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);

}
void startRecord(){
	int x = sqlite3_open_v2(path, &db,SQLITE_OPEN_READWRITE|SQLITE_OPEN_FULLMUTEX,NULL);

	char* error = 0;
		sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, &error);
	sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
	
}
void endRecord(){
		//sqlite3_close(db);
	char* sErrMsg;
	sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &sErrMsg);

}
void synapseEventRecord(regid_t core, regid_t local, tw_stime timestamp, int dest){
	static int row = 0;
	char *zErrMsg=0;
	char *sql = sqlite3_mprintf("insert into SynapseEvents ( \"localID\", \"coreID\", \"eventTime\", \"eventID\", \"sent\") values (%lu,%lu,%f,%i,%i);", local, core, timestamp, row, dest);
	int rc;


	rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
	row ++;


}

void recordNeuron(neuronState *n){
	static int row = 0;
	char *zErrMsg=0;
	char *sql = sqlite3_mprintf("insert into neurons (rowID, neuronID, dendCore,   dendGID, dendLocal, thresh, other) values (%i, %i, %i, %llu, %i, %i, %i)", row, n->neuronID, n->dendriteCore,  n->dendriteDest,n->dendriteLocalDest, n->threshold, n->threshold);
	int rc;
	rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
	row ++;
}
void finalClose() {
	sqlite3_close_v2(db);
}