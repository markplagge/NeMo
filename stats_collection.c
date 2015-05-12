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
const char* couchName = "tnt_benchmark_stats";
 char* sCu = "--silent -X POST http://128.213.23.52:5984/";
 char* midCu =  "  -H \"Content-Type: application/json\" -d '{";
 char* endCu = "}' >nul";
 char* path;
int buffSize = 16078;
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
	" (eventID INTEGER NOT NULL PRIMARY KEY,"
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
	char* cmd = sqlite3_mprintf("curl -X PUT http://128.213.23.52:5984/%s",couchName);
	pid_t pID = fork();
	if(pID == 0) {
		exit(system(cmd));
	} else{
		int status;
		wait(&status);
		sqlite3_free(cmd);
	}
}
void writeToCouchTS(char* eventType, int numTextParams, char* txtParamNames[] ,char*txtParams[], int numValParams, char* numParamNames[], uint64_t* numParams,double ts){
int status;	
	signal(SIGCHLD,SIG_IGN);
//	pid_t pID = fork();
//	if(pID == 0){	
	//assemble the couch string
	char* finalStr;
	char* cla = sqlite3_mprintf("%s%s%s",sCu,couchName,midCu);
	char* data = calloc(buffSize,sizeof(char));
		char* outs = calloc(buffSize, sizeof(char));
		char* outs_vals = calloc(buffSize, sizeof(char));
		for(int i = 0; i < numValParams; i ++) {
			strcat(outs, "\"");
			strcat(outs, numParamNames[i]);
			strcat(outs, "\",");
		}
		for(int i = 0; i < numValParams; i ++)
		{
			char* vm = sqlite3_mprintf("%llu", numParams[i]);
			strcat(outs_vals,vm);
			strcat(outs_vals,",");
			sqlite3_free(vm);
		}
	while(numTextParams > 0){
		numTextParams--;
		strcat(data,"\"");
		strcat(data,txtParamNames[numTextParams]);
		strcat(data,"\":\"");
		strcat(strcat(data,txtParams[numTextParams]),"\",");

	}
	char* datav = calloc(buffSize,sizeof(char));
	while(numValParams > 0){
		numValParams --;
		strcat(datav,"\"");
		strcat(datav,numParamNames[numValParams]);
		strcat(datav,"\":");
		char* val = sqlite3_mprintf("%llu",numParams[numValParams]);
		strcat(datav,val);
		strcat(datav,",");
		sqlite3_free(val);

	}


//	finalStr = sqlite3_mprintf("curl %s%s%s\"final\":\"complete\",\"simTimeStamp\":%d,\"eventType\":\"%s\"%s",cla,data,datav,ts,eventType,endCu);
//
		//InfluxDB tester:
finalStr = sqlite3_mprintf("curl %s%s%s\"final\":\"complete\",\"eventType\":\"%s\",\"eventTS\":%f%s",cla,data,datav,eventType,ts,endCu);
		char* fs2 = sqlite3_mprintf("curl --silent -X POST -d '[{"
			                            "\"name\":\"%s\",\"columns\":[\"time\",%s\"fin\"],"
			                            "\"points\":[[%f,%s 1]]"
			                            " }]' "
			                            " 'http://128.213.23.52:8086/db/tnt_bench_events/series?u=root&p=root' ", eventType, outs,ts, outs_vals);
	//now use curl to write to the configured couchdb.
		//int rv = 0;//system(finalStr);
		//int rv = 0;
		//int rv = system(finalStr); // write to couchdb
		int rv = system(fs2); // write event data to influxdb.
		//printf("\n\nWould output to crashdb - \n %s \n", fs2);
		rv = system(finalStr); //events to influx, full log to couch.
		//execlp("curl", "curl", finalStr, NULL);
		free(data);
		sqlite3_free(cla);
		sqlite3_free(finalStr);
		free(datav);
		sqlite3_free(fs2);
		free(outs_vals);
	//exit(rv);
		
	//else{ 
		//wait(&status);
}


void writeToCouch(char* eventType, int numTextParams, char* txtParamNames[] ,char*txtParams[], int numValParams, char* numParamNames[], uint64_t* numParams){
	signal(SIGCHLD, SIG_IGN);
	pid_t pID = fork();
//assemble the couch string
if(pID == 0){
	char* finalStr;

	char* cla = sqlite3_mprintf("%s%s%s",sCu,couchName,midCu);
	char* data = calloc(buffSize,sizeof(char));
	while(numTextParams > 0){
		numTextParams--;
		strcat(data,"\"");
		strcat(data,txtParamNames[numTextParams]);
		strcat(data,"\":\"");
		strcat(strcat(data,txtParams[numTextParams]),"\",");

	}
	char* datav = calloc(buffSize,sizeof(char));
	while(numValParams > 0){
		numValParams --;
		strcat(datav,"\"");
		strcat(datav,numParamNames[numValParams]);
		strcat(datav,"\":");
		char* val = sqlite3_mprintf("%llu",numParams[numValParams]);
		strcat(datav,val);
		strcat(datav,",");

	}

	finalStr = sqlite3_mprintf("curl %s%s%s\"final\":\"complete\",\"eventType\":\"%s\"%s",cla,data,datav,eventType,endCu);
	//test string command:
//	printf("CURL command would be :  %s\n",finalStr);



	//now use curl to write to the configured couchdb.
		int rv = system(finalStr);
		//int rv = 1;
		//execlp("curl", "curl", finalStr, NULL);
		free(data);
		sqlite3_free(cla);
		sqlite3_free(finalStr);
		free(datav);
		exit(rv);
		}
		
	else{ 
		//wait(&status);
}
}
void mapRecord( int type, char* typet, int localID, int coreID, int lpid, tw_lpid gid){
	static int val = 0;
	char* tpt = type == 0? "Neuron":"Synapse";
	char* sql = sqlite3_mprintf("INSERT INTO mappings (ID, type, typeImp, core, local, LPID, gID) VALUES (%i, %Q, %Q, %i, %i, %i, %llu);", val, tpt, typet, coreID, localID, lpid, gid);
	char* zErrMsg = 0;
	int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
	char* names[2] = {"typeImp","type"};
	char* vals[2] = {typet,tpt};
	char* nnames[5] = {"ID","core","local","LPID","gid"};
	uint64_t hvals[5] = {val,coreID,localID,lpid,gid};
	writeToCouch("mapping",2,names,vals,5,nnames,hvals);
	val ++;
}
void neuronEventRecord(tw_lpid neuronGlobal, regid_t core, regid_t local, regid_t fromSynapse, tw_stime timestamp, long postPot, char *send){
	static int row = 0;
	char *zErrMsg=0;
	char* sql = sqlite3_mprintf("INSERT INTO neuronEvent ( coreID, localID, eventTime, receivedSynapse, postFirePotential, eventType) VALUES (%i, %i, %f, %i, %i, %Q); ",core,local,timestamp,fromSynapse,postPot, send);
	int rc;
	// rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg); //removed, now using couch / influx
	int tp = 1;
	int np = 4;
	char* tps[1] = {"evt_text_desc"};
	char* tpsv[1] = {send};
	char* nps[5]={"gid", "coreID","localID","recvFromSyn","postFirePot"};
	uint64_t npsv[5]={neuronGlobal, core,	local,	fromSynapse, postPot};
	
	//exec curl
	writeToCouchTS("neuronEvent",tp,tps,tpsv,np,nps,npsv,timestamp);	
	sqlite3_free(sql);

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
void synapseEventRecord(tw_lpid gid, regid_t core, regid_t local, tw_stime timestamp, tw_lpid dest){
	static int row = 0;
	char *zErrMsg=0;
	char *sql = sqlite3_mprintf("insert into SynapseEvents ( \"localID\", \"coreID\", \"eventTime\", \"eventID\", \"sent\") values (%lu,%lu,%f,%i,%i);", local, core, timestamp, row, dest);
	int rc;
	//rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg); //removed, now using couch / influx.
	row ++;
	sqlite3_free(sql);
char* tps[5]={"gid","localID","coreID","eventID","sentTo"};
uint64_t tpsv[5]={gid,local,core,row,dest};
	writeToCouchTS("synapseEvent",0,NULL,NULL,4,tps,tpsv,timestamp);
} 
void recordNeuron(neuronState *n){
	static int row = 0;
	char *zErrMsg=0;
	char *sql = sqlite3_mprintf("insert into neurons (rowID, neuronID, dendCore,   dendGID, dendLocal, thresh, other) values (%i, %i, %i, %llu, %i, %i, %i)", row, n->neuronID, n->dendriteCore,  n->dendriteDest,n->dendriteLocalDest, n->threshold, n->threshold);
	int rc;
	//rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg); removed, now using couch /influx
	row ++;
	sqlite3_free(sql);
	char* nhds[6] = {"rowID", "neuronID", "dendCore","dendGID","dendLocal","thresh"};
	uint64_t tpsv[6] = {row,n->neuronID,n->dendriteCore, n->dendriteDest, n->dendriteLocalDest, n->threshold};
	writeToCouch("neuronStateRecord",0,NULL,NULL,6,nhds,tpsv);
}
void finalClose() {
	sqlite3_close_v2(db);
}
