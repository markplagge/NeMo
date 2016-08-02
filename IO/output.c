//
// Created by Mark Plagge on 8/2/16.
//
#include "output.h"

// POSIX File handles for non MPI IO file writing

FILE * outputFile;

FILE * neuronFireFile;


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


/** @defgroup MPI_IO_WRITE MPI File IO output data. Currently this is not used, but this is a skeleton for
 * collaborative IO if the POSIX multi process multi file technique kills performance.
 * @{
 */

// Neuron Fire Record MPI pointers and datatypes

MPI_File  *neuronFireFileMPI;

char * mpiFileName = "spike_events_mpi.csv";

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

//not sure if we need to buffer MPI writes, but if so this is the buffer for MPI.
neuronFireStruct * neuronFireBuffer[];

/**@}*/

/** @defgroup outputFileHandler File output handler functions. This group contains functions that initialize file
 * pointers and buffers for output file operations. Based on runtime and compile time flags, these functions init
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




/** @} */




/** @defgroup neuronFireRecord Neuron Fire Record Functions
 * These functions manage the neuron fire event record. These functions currently use POSIX file IO,
 * and generate one file per rank during runs. The buffers and files are initialized inside the initFiles() function,
 * contained in the more file output handler group.
 * @see{outputFileHandler}.
 * @{
 */

/** @}
 * */