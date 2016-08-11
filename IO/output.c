//
// Created by Mark Plagge on 8/2/16.
//
#include "output.h"

// POSIX File handles for non MPI IO file writing

FILE * outputFile;

FILE * neuronFireFile;
FILE * neuronFireFileBinary;

//Flags to check that output files are open:

bool neuronFireFileOpen;
bool outputFileOpen;



//Names of output files
char * neuronFireFinalFN;
char * neuronRankFN;
//char * neuronFireFileName = "fire_record"; <-- Global variable sets the name of the fire record.

//Global Memory Pool Position Counters
int neuronFirePoolPos = 0;
int neuronFireCompletedFiles = 0;

/* Text Mode Buffers */
char ** neuronFireBufferTXT;


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
}neuronFireStruct;


/** Binary buffer container for POSIX write */
neuronFireStruct * neuronFireBufferBIN;

/** @defgroup MPI_IO_WRITE MPI File IO output data.
 * Currently this is not used, but this is a skeleton for
 * collaborative IO if the POSIX multi process multi file technique kills performance.
 * @{
 */

// Neuron Fire Record MPI pointers and datatypes

MPI_File  *neuronFireFileMPI;

char * mpiFileName = "spike_events_mpi.dat";



//not sure if we need to buffer MPI writes, but if so this is the buffer for MPI.
neuronFireStruct * neuronFireBufferMPI;

/**@}*/

/** @defgroup neuronFireRecord Neuron Fire Record Functions
 * These functions manage the neuron fire event record. These functions currently use POSIX file IO,
 * and generate one file per rank during runs. The buffers and files are initialized inside the initFiles() function,
 * contained in the more file output handler group.
 * @see{outputFileHandler}.
 * @{
 */


void flushNeuron(){
	if(BINARY_OUTPUT){
		//handle binary output
		fwrite(neuronFireBufferBIN, neuronFirePoolPos , sizeof(neuronFireStruct),neuronFireFileBinary);
	}else {
		while (--neuronFirePoolPos > -1) {
			
			fprintf(neuronFireFile, "%s\n", neuronFireBufferTXT[neuronFirePoolPos]);
		}
		
	}
	//reset the memory pool counter.
	neuronFirePoolPos= 0;
}

void setNeuronNetFileName(){
	
		char * ext = BINARY_OUTPUT? ".dat" : ".csv";
		neuronRankFN = (char *) calloc(128, sizeof(char));
		sprintf(neuronRankFN, "%s_rank_%li%s",neuronFireFileName, g_tw_mynode,ext);
		neuronFireFinalFN = (char *) calloc(128, sizeof(char));
		sprintf(neuronFireFinalFN, "%s_final.csv", neuronFireFileName);
	
}

void saveNeuronFire(tw_stime timestamp, id_type core, id_type local, tw_lpid destGID){
	if (neuronFirePoolPos >= N_FIRE_BUFF_SIZE){
		flushNeuron();
	}
	if(BINARY_OUTPUT){
		
		neuronFireBufferBIN[neuronFirePoolPos].core = core;
		neuronFireBufferBIN[neuronFirePoolPos].dest = destGID;
		neuronFireBufferBIN[neuronFirePoolPos].timestamp = timestamp;
		neuronFireBufferBIN[neuronFirePoolPos].local = local;
		neuronFireBufferBIN[neuronFirePoolPos].ne = '|';
		
	}else {
		sprintf(neuronFireBufferTXT[neuronFirePoolPos], "%.30f,%i,%u,%llu",
		        timestamp, core, local, destGID);
	}
	
	++ neuronFirePoolPos;
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

void setFileNames(){
	if(SAVE_SPIKE_EVTS){
		setNeuronNetFileName();
	}
}

void initOutFiles(){
	setFileNames();
	if(SAVE_SPIKE_EVTS) {
		if(BINARY_OUTPUT) {
			neuronFireBufferBIN = (neuronFireStruct *) tw_calloc(TW_LOC,"OUTPUT",sizeof(neuronFireStruct),N_FIRE_BUFF_SIZE);
			neuronFireFile = fopen(neuronRankFN, "wb");
			
		}else{
			neuronFireBufferTXT = (char **) tw_calloc(TW_LOC, "OUTPUT", sizeof(char *),N_FIRE_BUFF_SIZE);
			
			for(int i = 0; i < N_FIRE_BUFF_SIZE; i ++) {
				neuronFireBufferTXT[i] = (char *) tw_calloc(TW_LOC, "OUTPUT", sizeof(char *), N_FIRE_LINE_SIZE);
			}
			neuronFireFile = fopen(neuronRankFN, "w");
		}
//
//        MPI_File_open(MPI_COMM_WORLD,mpiFileName,
//                      MPI_MODE_CREATE|MPI_MODE_WRONLY,MPI_INFO_NULL,neuronFireFileMPI);
//
//        MPI_File_set_atomicity(neuronFireFileMPI,1);
		
	}
	
}
void closeFiles(){
	flushNeuron();
	fclose(neuronFireFile);
//    MPI_File_close(neuronFireFileMPI);
	MPI_Barrier(MPI_COMM_WORLD); // wait for everyone to catch up.
	if(g_tw_mynode == 0) {
		
		FILE *finalout = fopen("neuron_spike_evts.csv", "w");
		fprintf(finalout, "timestamp,neuron_core,neuron_local,destGID\n");
		fclose(finalout);
	}
	
	
}


/** @} */




