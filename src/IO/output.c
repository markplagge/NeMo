//
// Created by Mark Plagge on 8/2/16.
//
#include "output.h"
//#include <pthread.h>
//#include <stdatomic.h>

#include <unistd.h>
//#include "../lib/c11t/c11threads.h"
// POSIX File handles for non MPI IO file writing

FILE *outputFile;

FILE *neuronFireFile;
FILE *neuronFireFileBinary;

//Flags to check that output files are open:

bool neuronFireFileOpen;
bool outputFileOpen;

//int N_FIRE_BUFF_SIZE = 32;
//Names of output files
char *neuronFireFinalFN;
char *neuronRankFN;
//char * NEURON_FIRE_R_FN = "fire_record"; <-- Global variable sets the name of the fire record.

//Global Memory Pool Position Counters
int neuronFirePoolPos = 0;
int neuronFireCompletedFiles = 0;

/* Text Mode Buffers */
char **neuronFireBufferTXT;


//multithreading fileIO for sim performance

/*
pthread_mutex_t *bufferLock;
typedef struct Queue
{
  //Implement this with two ints - one is the current
  //writing location, the other is the current reading location.
  //if they are equal....
  char * stk[N_FIRE_BUFF_SIZE];
  char * front;
  char * rear;
  int size;
  atomic_flag *isEmpty;
  atomic_flag *workDone;

}queue;

void enque(queue *s, char *data){
  // pushes and sets atomic flag to true - if it is set
  // blocks when queue is full
  while (s->size >= N_FIRE_BUFF_SIZE){
    sleep(1);
  }
  s->size ++;
  s->rear = (s->stk[s->size]);
  s->stk[s->size] = data;
  atomic_flag_clear(s->isEmpty);
}
int pop(queue *q, char *data){

  while(atomic_flag_test_and_set(q->isEmpty)){
    //IF GLOBAL THREAD SHUTDOWN HAPPENS,
    //QUIT
    sleep(1);
  }
  atomic_flag_clear(q->isEmpty);
  while(q->front){

  }

}
typedef struct {
  int buf[N_FIRE_BUFF_SIZE];

};
pthread_t writerThread;
int startWritingThread(){
  int result;
  result = pthread_mutex_init(bufferLock,NULL);

}

void fileWriter(){
//get access to bufffer list
  int status = pthread_mutex_lock(bufferLock);
  //check status
  //call flush neuron



} */

/** neuronFireStruct contains the in-memory representation of a neuron spike event. Used for binary
 * data saving of neuron events. This struct will be used both in MPI and in POSIX file IO. Currently, MPI-IO is
 * not implemented and neruron spikes are saved as text, but in the future spikes will be saved as binary.
 */
typedef struct NeuronFireStruct {
  tw_stime timestamp;
  id_type core;
  id_type local;
  tw_lpid dest;
  char ne;
} neuronFireStruct;

/** Binary buffer container for POSIX write */
neuronFireStruct *neuronFireBufferBIN;

/** @defgroup MPI_IO_WRITE MPI File IO output data.
 * Currently this is not used, but this is a skeleton for
 * collaborative IO if the POSIX multi process multi file technique kills performance.
 * @{
 */

// Neuron Fire Record MPI pointers and datatypes

MPI_File *neuronFireFileMPI;

char *mpiFileName = "spike_events_mpi.dat";

//not sure if we need to buffer MPI writes, but if so this is the buffer for MPI.
neuronFireStruct *neuronFireBufferMPI;

/**@}*/

/** @defgroup neuronFireRecord Neuron Fire Record Functions
 * These functions manage the neuron fire event record. These functions currently use POSIX file IO,
 * and generate one file per rank during runs. The buffers and files are initialized inside the initFiles() function,
 * contained in the more file output handler group.
 * @see{outputFileHandler}.
 * @{
 */


void flushNeuron() {
  if (BINARY_OUTPUT) {
    //handle binary output
    fwrite(neuronFireBufferBIN, neuronFirePoolPos, sizeof(neuronFireStruct), neuronFireFileBinary);
  } else {
    while (--neuronFirePoolPos > -1) {

      fprintf(neuronFireFile, "%s\n", neuronFireBufferTXT[neuronFirePoolPos]);
    }

  }
  //reset the memory pool counter.
  neuronFirePoolPos = 0;
}

void setNeuronNetFileName() {

  char *ext = BINARY_OUTPUT ? ".dat" : ".csv";
  neuronRankFN = (char *) calloc(128, sizeof(char));
  sprintf(neuronRankFN, "%s_rank_%li%s", NEURON_FIRE_R_FN, g_tw_mynode, ext);
  neuronFireFinalFN = (char *) calloc(128, sizeof(char));
  sprintf(neuronFireFinalFN, "%s_final.csv", NEURON_FIRE_R_FN);

}

void saveNeuronFire(tw_stime timestamp, id_type core, id_type local, tw_lpid destGID, long destCore,
                    long destLocal, unsigned int isOutput) {
  if (neuronFirePoolPos >= N_FIRE_BUFF_SIZE) {
    flushNeuron();
  }
  if (BINARY_OUTPUT) {

    neuronFireBufferBIN[neuronFirePoolPos].core = core;
    neuronFireBufferBIN[neuronFirePoolPos].dest = destGID;
    neuronFireBufferBIN[neuronFirePoolPos].timestamp = timestamp;
    neuronFireBufferBIN[neuronFirePoolPos].local = local;
    neuronFireBufferBIN[neuronFirePoolPos].ne = '|';

  } else {

    sprintf(neuronFireBufferTXT[neuronFirePoolPos], "%.30f,%i,%u,%llu,%l,%l,%u",
            timestamp, core, local, destGID, destCore, destLocal, isOutput);
  }

  ++neuronFirePoolPos;
}

/** @} */


/** @defgroup outputFileHandler File output handler functions.
 * This group contains functions that initialize file pointers and buffers for output file operations.
 * Based on runtime and compile time flags, these functions init
 * the buffers needed for output, along with file pointers.
 *
 * ## Global Flags Used:
 *
 *  - #SAVE_SPIKE_EVTS - Flag that determines if neuron spike events are to be saved.
 *  - #N_FIRE_BUFF_SIZE - Number of spike events that are stored in memory before being saved to disk.
 *  - #N_FIRE_LINE_SIZE
 *	@secreflist
 *	\refitem SAVE_SPIKE_EVTS
 *	\refitem N_FIRE_BUFF_SIZE
 *	\refitem N_FIRE_LINE_SIZE
 *	\refitem neuronFireFileName
 *	\refitem fileNames
 *	@endsecreflist
 * @{
 */

void setFileNames() {
  if (SAVE_SPIKE_EVTS || SAVE_OUTPUT_NEURON_EVTS) {
    setNeuronNetFileName();
  }
}

void initOutFiles() {
  setFileNames();
  int tv = N_FIRE_BUFF_SIZE;
  if (SAVE_SPIKE_EVTS || SAVE_OUTPUT_NEURON_EVTS) {
    if (BINARY_OUTPUT) {
      neuronFireBufferBIN = (neuronFireStruct *) tw_calloc(TW_LOC, "OUTPUT", tv, sizeof(neuronFireStruct));
      neuronFireFile = fopen(neuronRankFN, "wb");

    } else {
      neuronFireBufferTXT = (char **) tw_calloc(TW_LOC, "OUTPUT", tv, sizeof(char *));

      for (int i = 0; i < N_FIRE_BUFF_SIZE; i++) {
        neuronFireBufferTXT[i] = (char *) tw_calloc(TW_LOC, "OUTPUT", tv, sizeof(char *));
      }
      neuronFireFile = fopen(neuronRankFN, "w");
      fprintf(neuronFireFile, "timestamp,core,local,destGID,destCore,destNeuron,isOutput?");
    }
//
//        MPI_File_open(MPI_COMM_WORLD,mpiFileName,
//                      MPI_MODE_CREATE|MPI_MODE_WRONLY,MPI_INFO_NULL,neuronFireFileMPI);
//
//        MPI_File_set_atomicity(neuronFireFileMPI,1);

  }

}
void closeFiles() {
  flushNeuron();
  fclose(neuronFireFile);
//    MPI_File_close(neuronFireFileMPI);
  MPI_Barrier(MPI_COMM_WORLD); // wait for everyone to catch up.
  if (g_tw_mynode == 0) {

    FILE *finalout = fopen("neuron_spike_evts.csv", "w");
    fprintf(finalout, "timestamp,neuron_core,neuron_local,destGID\n");
    fclose(finalout);
  }

}


/** @} */




