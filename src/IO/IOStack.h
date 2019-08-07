//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_IOSTACK_H
#define NEMO_IOSTACK_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../globals.h"
#include "../lib/simclist/simclist.h"
#include "../nemo_config.h"
#include "../lib/itrlve.h"
//#include "../lib/lua.h"
//#include "../lib/lualib.h"
//#include "../lib/lauxlib.h"
#include <lauxlib.h>
#include <lualib.h>
#include "../../libs/model_reader/include/model_reader_wrapper.h"
/** Macro wrappers for interleaving time/axonids */
#define EXTIME(x) EXHIGH(x)
#define EXAXON(x) EXLOW(x)

uint64_t interleave(uint32_t time, uint32_t axid);

/** External filename variables */

extern char *NEURON_FIRE_R_FN;

#define SAVE_ALL_NEURON_PARAMS 1
#define DBG_MODEL_MSGS 0
enum modelReadMode {
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
struct tnCSV {
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


/* Spike Loading Functions (generics) */
/** Not used in NeMo2 */
int getSpikesFromAxon(void *timeList, id_type core, id_type axonID);
/**
 * Core-wise spike loading. Synapse calls this function with its core and and inited list. returns num of
 * spikes and a populated list.
 * @param timeList A list populated with interleaved time->axonID structs
 * @param core the coreID of the synapse
 * @return the number of spikes in the input file.
 */
int getSpikesFromSynapse(void *timeList, id_type core);
/**
 * Core-wise spike counter. Synapse calls this function to see if there are spikes in the input file for this
 * particular core.
 * @param core The local coreID of the synapse.
 * @return number of spikes
 */
int getNumSpikesForCore(id_type core);

int spikeFromSynapseComplete(void *timeList);

int spikeFromAxonComplete(void *timeList);

int openSpikeFile();
int closeSpikeFile();

int loadSpikesFromFile(char *filename);

spikeElem *getSpike(long destCore, long destAxon);

int getSpikeCount();

void testSpikes();


/** @}**/
/** @defgroup spout Spike Output Functions @{ */
void initOutFiles();
void closeFiles();

//void saveEvent(tw_stime timestamp, char sourceType, id_type core, id_type local,
//               id_type destCore, id_type destLocal);



void saveNeuronFire(tw_stime timestamp, id_type core, id_type local, tw_lpid destGID, long destCore,
                    long destLocal, unsigned int isOutput);
void openOutputFiles(char *outputFileName);
void initDataStructures(int simSize);
void closeOutputFiles();
void saveIndNeuron(void *n);
void saveNetworkStructure();

/** @} */

/** @defgroup modelReading @{ */
typedef enum P_TYPES {
  LONGARRAY,
  UNSIGNEDLONGARRAY,
  LONG,
  UNSIGNEDLONG
} p_types;

typedef struct NeuroConnect {
  uint64_t myCoreID;
  uint64_t neuronID;
  uint64_t destCore;
  uint64_t destAxon;
  uint64_t *conArray;
  uint64_t *weights;
} neuroCon;
typedef struct NeuroSpike {
  uint64_t myCoreID;
  uint64_t neuronID;
  uint64_t destCore;
  uint64_t destAxon;
  //long long double destTime;
} neuroSpike;
int writeNeuronConnections(neuroCon *neuronData);
int writeNeuronSpike(neuroSpike *spikeInfo);

void saveNeuronNetworkStructure(void *n);
void saveNetworkStructureMPI();
void saveNeuronPreRun();

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
int populateNeuron(long coreID, long localID, void *neuron, char nt[2], char **paramList, p_types *paramTypes);

/** lookupNeuron finds a neuron in the config file, and pushes it's table to the top of the lua stack.
 * Call this function first, then call lPushParam() followed by lGetParam()
 * @param coreID
 * @param localID
 * @param nt NeuronType: can be "TN"
 * @return -1 if not found, 0 if found.
 */
int lookupAndPrimeNeuron(long coreID, long localID, char *nt);

/**
 * Once a neuron is selected, this pushes a parameter from the table to the top of the stack .
 * Run this after lookupNeuron()
 * @param paramName
 */
void lPushParam(char *paramName);

/**
 * onece a parameter is pushed (see lpushParam()), this extracts the parameter.
 * If parameter is an array, returns the number of elements placed in the array specified by arrayParam.
 * Otherwise, returns the value from the config file (that was specified in the lPushParam() function.
 * @param isArray
 * @param arrayParam
 * @return
 */

long long int lGetParam(int isArray, long *arrayParam);

/**
 * Returns a specific global model configuration parameter.
 * Currently, (for optimization reasons), these are not really used. Most global configuration
 * settings are handled at build time.
 * @param modelParamName
 * @return
 */
long getModelParam(char *modelParamName);

/**
 * a helper function that calls lPushParam then lGetParam.
 * @param paramName
 * @param isArray
 * @param arrayParam
 * @return If no neuron is found -1 - otherwise it returns the value, or the length of values found in the array.
 */
long lGetAndPushParam(char *paramName, int isArray, long *arrayParam);

/**
 * A model read error helper function */

void getModelErrorInfo(int ncore, int nlocal, char *ntype, char *paramName, int errorno);

/**
 * Prepares the model file for loadiing. Loads the file into memory and stores it in a list.
 * @param filename the model filename, must be a NeMo CSV.
 * @param maxNeurons The maximum number of neurons in the model file. Is an upper bound on the list size. If set to -1,
 * will be estimated from the number of lines in the file.
 */
void initModelInput(unsigned long maxNeurons);

/**
 * uses a lua table to lookup the nemo config variable name from a TN Name
 * @param nemoName
 * @return
 */
char *luT(char *nemoName);

void clearNeuron(int curCoreID, int curLocalID);

/**
 * closeModelInput closes the LUA parser after the model has been loaded.
 * Do this once the neurons have be initialized.
 */
void closeModelInput();
void clearStack();
void closeLua();
/** @} */

/** @defgroup new_io New C++ backed IO stack functions */

void loadNeuronFromJSON(id_type neuronCore, id_type neuronLocal,tn_neuron_state *n);
void initJSON(char *jsonFilename);
void closeJSON();

/**
 * for reading in TN neuron structs saved to file. Currently this binary system only handles TN LPs saved from
 * the JSON parser.
 * @param binFileName
 * @return
 */
int openBinaryModelFile(char * binFileName);
/**
 * Initializes the library of neuron structs from the binary file.
 * @return the number of neurons loaded.
 */
long setupBinaryNeurons();
/**
 * initializes the given neuron, if it exists in the neuron library.
 * @param neuronCore
 * @param neuronLocal
 * @param n
 */
bool loadNeuronFromBIN(id_type neuronCore, id_type neuronLocal, tn_neuron_state *n);


/**
 * Closes the binary file and frees the library of neurons.
 */
void closeBinaryModelFile();

/** @todo: Deug functions - can remove once missing spikes are found */
void saveNeuronFireDebug(tw_stime timestamp, id_type core, id_type local, tw_lpid destGID, long destCore,
                         long destLocal, unsigned int isOutput);
void debug_neuron_connections(tn_neuron_state *n,tw_lp *lp);



/**@}*/
#endif //NEMO_IOSTACK_H
