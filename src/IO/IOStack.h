//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_IOSTACK_H
#define NEMO_IOSTACK_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../globals.h"
#include "../lib/simclist.h"
#include "../nemo_config.h"
//#include "../lib/lua.h"
//#include "../lib/lualib.h"
//#include "../lib/lauxlib.h"
/** External filename variables */

extern char * NEURON_FIRE_R_FN	;
extern char * NETWORK_CFG_FN 	;
extern char * SPIKE_IN_FN 		;
#define SAVE_ALL_NEURON_PARAMS 1
#define DBG_MODEL_MSGS 1
enum modelReadMode{
    START_READ,
    MODEL_HDR,
    N_TYPE,
    N_CORE,
    N_LOCAL,
    N_CONNECTIVITY,
    N_AXONTYPES,
    N_SGI,
    N_SP,
    N_BV,
    N_PARAMS
};
/**
 * struct tnCSV stores the values read in via the multi-file system.
 */
struct tnCSV{
    unsigned int fldNumber;
    enum modelReadMode rm;
    long coreID;
    long localID;
    char connectivity[AXONS_IN_CORE];
    char axontypes[AXONS_IN_CORE];
    char sgi[NUM_NEURON_WEIGHTS];
    char sp[NUM_NEURON_WEIGHTS];
    char bv[NUM_NEURON_WEIGHTS];
    char params[128];
};

/** @defgroup spin Spike Input Functions @{ */
/**
 * opens the spike file.
 * @param filename Optional - if provided will overrride the global variable.
 * @return status - 0 if good.
 */
int openSpikeFile(char * filename);
int closeSpikeFile();

int loadSpikesFromFile(char * filename);

spikeElem * getSpike(long destCore, long destAxon);

int getSpikeCount();

void testSpikes();


/** @}**/
/** @defgroup spout Spike Output Functions @{ */
void initOutFiles();
void closeFiles();

//void saveEvent(tw_stime timestamp, char sourceType, id_type core, id_type local,
//               id_type destCore, id_type destLocal);



void saveNeuronFire(tw_stime timestamp, id_type core, id_type local, tw_lpid destGID);
void openOutputFiles(char * outputFileName);
void initDataStructures(int simSize);
void closeOutputFiles();
void saveIndNeuron(void *n);
/** @} */

/** @defgroup modelReading @{ */
typedef enum P_TYPES{
	LONGARRAY,
	UNSIGNEDLONGARRAY,
	LONG,
	UNSIGNEDLONG
}p_types;
/**
 * More complex auto populate neuron function. Given the coreID, localID, a state strut, and some defs,
 * will blindly fill in the struct with data.
 * @param coreID  Neuron COREID
 * @param localID neuron LOCALID
 * @param neuron A preallocated neuron structure
 * @param nt The neuron type -- can be "TN"
 * @param paramList A list, given in order, of parameters to look for and load into the struct
 * @param paramTypes A list of paramter types.
 * @return 0 if everything is fine. -1 if fail.
 */
int populateNeuron(long coreID, long localID, void * neuron, char nt[2], char ** paramList, p_types* paramTypes);


/** lookupNeuron finds a neuron in the config file, and pushes it's table to the top of the lua stack.
 * Call this function first, then call lPushParam() followed by lGetParam()
 * @param coreID
 * @param localID
 * @param nt NeuronType: can be "TN"
 * @return
 */
int lookupAndPrimeNeuron(long coreID, long localID, char * nt);

/**
 * Once a neuron is selected, this pushes a parameter from the table to the top of the stack .
 * Run this after lookupNeuron()
 * @param paramName
 */
void lPushParam(char* paramName);

/**
 * onece a parameter is pushed (see lpushParam()), this extracts the parameter.
 * If parameter is an array, returns the number of elements placed in the array specified by arrayParam.
 * Otherwise, returns the value from the config file (that was specified in the lPushParam() function.
 * @param isArray
 * @param arrayParam
 * @return
 */

long lGetParam(int isArray, long * arrayParam);



/**
 * a helper function that calls lPushParam then lGetParam.
 * @param paramName
 * @param isArray
 * @param arrayParam
 * @return If no neuron is found -1 - otherwise it returns the value, or the length of values found in the array.
 */
long lGetAndPushParam(char * paramName, int isArray, long * arrayParam);



/**
 * Prepares the model file for loadiing. Loads the file into memory and stores it in a list.
 * @param filename the model filename, must be a NeMo CSV.
 * @param maxNeurons The maximum number of neurons in the model file. Is an upper bound on the list size. If set to -1,
 * will be estimated from the number of lines in the file.
 */
void initModelInput(unsigned long maxNeurons);

/** @} */

#endif //NEMO_IOSTACK_H
