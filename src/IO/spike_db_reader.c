//
// Created by Mark Plagge on 11/28/17.
//


#include "spike_db_reader.h"

sqlite3 *spikedbFile;
sqlite3 *spikedb;
int spikedb_isopen = 0;

/** @defgroup sqlbuf SQL buffers and queries @{ */
//SQL Query building blocks:

char *q1 = "SELECT   input_spikes.time\n"
    "FROM     input_spikes\n"
    "WHERE    ( input_spikes.axon = ";
char *q2 = " ) AND ( input_spikes.core = ";
char *q3 = ")";
char *cntq = "select count(input_spikes.time)\n"
    "FROM input_spikes\n"
    "WHERE (input_spikes.axon=";

/**
 * Core sql query building block 1
 */
char *coreq1 = "SELECT \"input_spikes\".\"time\", \"input_spikes\".\"axon\"\n"
    "FROM   \"input_spikes\"\n"
    "WHERE \"input_spikes\".\"core\" = ";
/**
 * core sql query building block 1 - counter.
 */
char *corect1 = "SELECT count(input_spikes.time)\nFROM input_spikes\nWHERE input_spikes.core = ";

/**
 * String query buffers
 */
char *fullQ;
char *coreSTR;
/**
 * @}
 */

/*
** This function is used to load the contents of a database file on disk
** into the "main" database of open database connection pInMemory, or
** to save the current contents of the database opened by pInMemory into
** a database file on disk. pInMemory is probably an in-memory database,
** but this function will also work fine if it is not.
**
** Parameter zFilename points to a nul-terminated string containing the
** name of the database file on disk to load from or save to. If parameter
** isSave is non-zero, then the contents of the file zFilename are
** overwritten with the contents of the database opened by pInMemory. If
** parameter isSave is zero, then the contents of the database opened by
** pInMemory are replaced by data loaded from the file zFilename.
**
** If the operation is successful, SQLITE_OK is returned. Otherwise, if
** an error occurs, an SQLite error code is returned.
*/
int loadOrSaveDb(sqlite3 *pInMemory, const char *zFilename, int isSave) {
  int rc;                   /* Function return code */
  sqlite3 *pFile;           /* Database connection opened on zFilename */
  sqlite3_backup *pBackup;  /* Backup object used to copy data */
  sqlite3 *pTo;             /* Database to copy to (pFile or pInMemory) */
  sqlite3 *pFrom;           /* Database to copy from (pFile or pInMemory) */

  /* Open the database file identified by zFilename. Exit early if this fails
  ** for any reason. */
  rc = sqlite3_open(zFilename, &pFile);
  if (rc==SQLITE_OK) {

    /* If this is a 'load' operation (isSave==0), then data is copied
    ** from the database file just opened to database pInMemory.
    ** Otherwise, if this is a 'save' operation (isSave==1), then data
    ** is copied from pInMemory to pFile.  Set the variables pFrom and
    ** pTo accordingly. */
    pFrom = (isSave ? pInMemory : pFile);
    pTo = (isSave ? pFile : pInMemory);

    /* Set up the backup procedure to copy from the "main" database of
    ** connection pFile to the main database of connection pInMemory.
    ** If something goes wrong, pBackup will be set to NULL and an error
    ** code and message left in connection pTo.
    **
    ** If the backup object is successfully created, call backup_step()
    ** to copy data from pFile to pInMemory. Then call backup_finish()
    ** to release resources associated with the pBackup object.  If an
    ** error occurred, then an error code and message will be left in
    ** connection pTo. If no error occurred, then the error code belonging
    ** to pTo is set to SQLITE_OK.
    */
    pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
    if (pBackup) {
      (void) sqlite3_backup_step(pBackup, -1);
      (void) sqlite3_backup_finish(pBackup);
    }
    rc = sqlite3_errcode(pTo);
  }

  /* Close the database connection opened on database file zFilename
  ** and return the result of this function. */
  (void) sqlite3_close(pFile);
  return rc;
}

long execCountCallback(void *nu, int argc, char **argv, char **azColName) {

  printf("%s = %s\n", azColName[0], argv[0] ? argv[0] : "NULL");
  return strtol(argv[0], NULL, 10);
}
int spikeQueryCallback(void *splist, int argc, char **argv, char **colName) {
  list_t *spikelist = (list_t *) splist;
  for (int i = 0; i < argc; i++) {

    list_append(spikelist, strtol(argv[i], NULL, 10));
  }
  /** @todo move *splist to a global value if possible - we don't really need to calloc it every time... */
  return argc;

}

int testImport() {
  return 2;
}

char *genSelectQuery(id_type axon, id_type core) {
  char *query = calloc(sizeof(char), 1024);
  sprintf(query, "%s %lli %s %lli %s", q1, axon, q2, core, q3);
  return query;
}

char *genCountQuery(id_type axon, id_type core) {
  char *query = calloc(sizeof(char), 1024);
  sprintf(query, "%s %lli %s %lli %s", cntq, axon, q2, core, q3);
  return query;
}

/**
 * Helper function for core query building. Returns an SQL query that will either count number of spikes or
 * return spikes. Sets the value in the global query buffers declared above (fullQ and coreSTR)
 * @param type 0 is count query, 1 is normal query.
 * @param core the coreID for the query
 *
 */
void assembleCoreQuery(int type, id_type core) {

  fullQ[0] = '\0';
  char *p = fullQ;
  coreSTR[0] = '\0';
  char *n = coreSTR;
  sprintf(coreSTR, "%llu", core);

  if (type) {
    mystrcat(p, coreq1);
    mystrcat(p, coreSTR);
    mystrcat(p, ";");
  } else {
    mystrcat(p, corect1);
    mystrcat(p, coreSTR);
    mystrcat(p, ";");
  }
}
int dbct;

static int coreCountCB(void *userData, int c_num, char **c_vals, char **c_names) {
  dbct = atoi(c_vals[0]);
  return 0;
}
static int coreQueryCB(void *userData, int c_num, char **c_vals, char **c_names) {

  list_t *splist = (list_t *) userData;
  uint32_t time = (uint32_t) strtol(c_vals[0], NULL, 10);
  uint32_t axid = (uint32_t) strtol(c_vals[1], NULL, 10);
  uint64_t val = interleave(time, axid);
  list_append(splist, &val);
//list_append(splist, strtol(c_vals[i], NULL, 10));
  dbct += 1;
  return 0;
}
char *errmsg;

int doesCoreHaveSpikesDB(id_type core) {
  //query stored in fullQ.
  assembleCoreQuery(1, core);
  errmsg = calloc(sizeof(char), 1024);
  int rc;
  sqlite3_stmt *res;
  rc = sqlite3_prepare_v2(spikedb, fullQ, -1, &res, 0);
  //rc = sqlite3_exec(spikedb, fullQ, coreCountCB, NULL, errmsg);
  if (rc!=SQLITE_OK) {
    TH
    printf("\n\nSQLITE error\n");
    printf("SQL ERROR TRACKING \n errmsg pre: %s \n\n. Error Code %i\n", sqlite3_errmsg(spikedb), rc);
    tw_error(TW_LOC, "\nSQL query: %s \n Core: %llu", fullQ, core);
  }
  int cntr = 0;
  if (sqlite3_step(res)==SQLITE_ROW) {
    cntr++;
  }
  free(errmsg);
  return cntr;
}



/**
 *Function that populates a linked list with input spike information:
 * Spikes are stored as a single 64 bit integer using the interleave function.
 * @param timeList An initialized simclist linked list that stores unsigned 64 bit ints
 * @param core The core that spikes should be for
 * @return error message.
 */

int getSpikesSynapseDB(void *timeList, id_type core) {
  assembleCoreQuery(1, core);
  errmsg = calloc(sizeof(char), 1024);
  sqlite3_stmt *res;
  int rc = sqlite3_prepare_v2(spikedb, fullQ, -1, &res, 0);
  if (rc!=SQLITE_OK) {

    tw_error(TW_LOC, "SQL Error - unable to execute statement %s\n", sqlite3_errmsg(spikedb));
  }
  int cntr = 0;
  //int step = sqlite3_step(res);
  list_t *splist = (list_t *) timeList;
  while (sqlite3_step(res)==SQLITE_ROW) {
    uint32_t time = sqlite3_column_int(res, 0);
    uint32_t axon = sqlite3_column_int(res, 1);
    uint64_t val = interleave(time, axon);
    list_append(splist, &val);
    dbct += 1;

  }
  free(errmsg);
  return dbct;


  //int rc = sqlite3_exec(spikedb, fullQ, coreQueryCB,timeList, errmsg);
  if (rc!=SQLITE_OK) {
    TH
    printf("\n\nSQLITE error in full query\n");
    STT("Message: %s", errmsg);
    tw_error(TW_LOC, "\nSQL query: %s \n Core: %llu", fullQ, core);
  }
  free(errmsg);
  return dbct;

}

/**
 * queries the sqlite db for a spike for axom.core. MessageData is populated with valid
 * info.
 * @param M - list pointer. Will be initialized in this function with n elements, where n is the number of elements found for this axon.
 *
 * @param coreID The core id.
 * @param axonID The local axon ID (core,axon notation)
 * @return 0 if spike not found, otherwise the number of elements found.
 */
int getSpikesFromAxonSQL(void *M, id_type coreID, id_type axonID) {

//    char * query = genSelectQuery(axonID, coreID);
//    char * countQuery = genCountQuery(axonID, coreID);

  list_t *spikelist = (list_t *) M;

  char *err_msg;
  sqlite3_stmt *res;
  int rc = 0;

  char *query = "SELECT   input_spikes.time\n"
      "FROM     input_spikes\n"
      "WHERE    ( input_spikes.axon = ?  ) AND ( input_spikes.core = ?  );";
  rc = sqlite3_prepare_v2(spikedb, query, -1, &res, 0);
  if (rc==SQLITE_OK) {
    sqlite3_bind_int64(res, 1, axonID);
    sqlite3_bind_int64(res, 2, coreID);
  } else {
    tw_error(TW_LOC, "SQL Error - unable to execute statement: Details: %s\n", sqlite3_errmsg(spikedb));

  }
  int cntr = 0;
  int step = sqlite3_step(res);
  if (step==SQLITE_ROW) {

    list_attributes_copy(spikelist, list_meter_int64_t, 1);

    while (step==SQLITE_ROW) {
      long sptime = sqlite3_column_int64(res, 0);
      list_append(spikelist, &sptime);
      cntr++;
      step = sqlite3_step(res);
    }
  }
  sqlite3_finalize(res);
  return cntr;

}

/**
 * queries the sqlite db for a spike for axon,core. returns 0 if no spike found, otherwise returns 1
 * @param coreID
 * @param axonID
 * @return
 */
int checkIfSpikesExistForAxon(id_type coreID, id_type axonID) {

}

int connectToDB(char *filename) {
  int st = -999;
  if (spikedb_isopen==0) {
    st = sqlite3_open(":memory:", &spikedb);
    //st = sqlite3_open_v2(filename, &spikedbFile, SQLITE_OPEN_READONLY|SQLITE_OPEN_NOMUTEX, NULL);
    //st = sqlite3_open_v2(filename,&spikedbFile, SQLITE_OPEN_READONLY, NULL);
    if (st) {
      tw_error(TW_LOC, "Can't open database: %s\n", sqlite3_errmsg(spikedb));

    }
    st = loadOrSaveDb(spikedb, filename, 0);
    if (st) {
      if (g_tw_mynode==0) {
        printf("\n\nDisk based sql backup enabled.");
        st = sqlite3_close(spikedb);
        st = sqlite3_open_v2(filename, &spikedb, SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX, NULL);
        if (st) {
          tw_error(TW_LOC, "Database open error: Code: %i \n Message %s\n filename: %s\n",
                   st, sqlite3_errmsg(spikedb), filename);
        }
      }
    }
    spikedb_isopen = 1;
  }
  return st;

}
int closeDB(char *filename) {
  int st = 0;
  if (spikedb_isopen==1) {
    st = sqlite3_close_v2(spikedb);
    spikedb_isopen = 0;
  }

  return (st==SQLITE_OK);
}
/** @defgroup spin Spike Input Functions @{ */
/**
 * Override - generic function to get spike list from Axon ID. This function uses SQLITE.
 * @param timeList A null array - initalized when spikes are found by this function.
 * @param core Axon's core ID
 * @param axonID  Axon's local ID
 * @return The number of spikes found for this axon.
 */
int getSpikesFromAxon(void *timeList, id_type core, id_type axonID) {
  return getSpikesFromAxonSQL(timeList, core, axonID);
}

/**
 * Override - closes the spike DB file. Generic function header so I can swap out sql for some other IO scheme
 * @param timeList
 * @return
 */
int spikeFromAxonComplete(void *timeList) {
  list_destroy((list_t *) timeList);
}
/**
 * Override - generic function - opens and inits the spike file. Here opens connection to the SQL database
 * @return
 */
int openSpikeFile() {
  connectToDB(SPIKE_FILE);
  fullQ = (char *) calloc(sizeof(char), 2048);
  coreSTR = (char *) calloc(sizeof(char), 1024);
}
/**
 * Override -- generic function - closes the spike file. Here closes the SQL database.
 * @return
 */
int closeSpikeFile() {
  return closeDB(SPIKE_FILE);
}

/**
 * Override - generic function caller that uses SQL for the spikes.
 *
 *
 */
int getNumSpikesForCore(id_type core) {
  return doesCoreHaveSpikesDB(core);
}

int getSpikesFromSynapse(void *timeList, id_type core) {
  return getSpikesSynapseDB(timeList, core);
}