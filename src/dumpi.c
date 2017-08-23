//
// Created by Mark Plagge on 8/18/17.
//

#include <printf.h>
#include "dumpi.h"
#include "ross.h"
#include "mapping.h"
/*
 * Set up datatypes for the MPI send/rcv cdoe
 */
int COUNT = 2; // One count for the COREid, one for the neuron local id.
int DTYPE = 11; // dtype  for the DUMPI file.
int COMM = 4;
int TAG = 0;
//unsigned int NUM_CHIPS_IN_SIM = CORES_IN_SIM / CORES_IN_CHIP;
//unsigned int CHIPS_PER_RANK; = g_tw_npe; //! Sets the number of chips per MPI rank for sim




double WALL_OFFSET = 0.001; //! Some sort of offset for the wall clock time - what is a good value?
double CPU_OFFSET = 0.000001; //! The CPU time for recv. messages.

double NEURO_CORE_CLOCK = 1000; //! Neuromorphic core speed (cycles / second).

/**
 * Converts the chip ID to an MPI rank.
 * @param chipID
 * @return
 */
size_type chipToRank(size_type chipID){
	long NUM_SIM_RANKS = NUM_CHIPS_IN_SIM;
#ifdef DEBUG
    if ((chipID / CHIPS_PER_RANK) > NUM_SIM_RANKS){
        printf("Error in chip to rank calculation \n");
        printf("Chip: %lu - CPR: %i - RANKS: %i - CALC %lu", chipID,
               CHIPS_PER_RANK, NUM_SIM_RANKS, chipID / NUM_SIM_RANKS);
    }
#endif
    return chipID / CHIPS_PER_RANK;
}

size_type coreToChip(size_type coreID){
    return coreID / CORES_IN_CHIP;

}

size_type coreToRank(size_type coreID){
    return chipToRank(coreToChip(coreID));
}

/**@{ */
/**
 * getWallStart returns the wallStart param for dumpi
 * @param twSendTime
 * @return
 */

double getWallStart(double twSendTime){
	tw_wtime *t;
	double ctime = tw_wall_to_double(t);
    return ctime + (arc4random() * WALL_OFFSET);
}

/**
 * getWallEnd returns the  wallEnd param for dumpi.
 * @param twSendTime
 * @return
 */
double getWallEnd(double twSendTime){
    return (getWallStart(twSendTime) + WALL_OFFSET);
}

/**
 * getCPUStart returns the cpuStart param for dumpi.
 * @param twSendTime
 * @return
 */
double getCPUStart(double twSendTime){
    return (twSendTime * CPU_OFFSET ) + (arc4random() * CPU_OFFSET);
}

/**
 * getCPUEnd returns the cpuEnd param for dumpi.
 * @param twSendTime
 * @return
 */
double getCPUEnd(double twSendTime){
    return getCPUStart(twSendTime) + CPU_OFFSET;
}

char * generateMsg(long sourceChip, long destChip, double twTimeSend,
                   char* type){
    char * outStr= calloc(sizeof(char), 128); // buffer
    sourceChip = chipToRank(sourceChip);
    destChip = chipToRank(destChip);
    int rv = sprintf(outStr, "%s %li %li %f %f %f %f %i %i %i 0\n",
                     type, sourceChip, destChip, getWallStart(twTimeSend),
                     getWallEnd(twTimeSend), getCPUStart(twTimeSend),
                     getCPUEnd(twTimeSend), COUNT, DTYPE, COMM, TAG);
	//virtual ranks are interleaved - add a "running-on" feature for script extraction at the start of the line
	rv = sprintf(outStr, "%li,%s",sourceChip,outStr);
    return outStr;
}
char * generateIsend(long sourceChip, long destChip, double twTimeSend){

    return generateMsg(sourceChip,destChip,twTimeSend,"MPI_Isend");
}
char * generateIrecv(long sourceChip, long destChip, double twTimeSend){
    return generateMsg(sourceChip, destChip, twTimeSend, "MPI_Irecv");
}

bool isDestInterchip(id_type core1, id_type core2){

    return coreToRank(core1) != coreToRank(core2);
}


void saveMPIMessage(id_type sourceCore, id_type destCore, double twTimeSend,
                    FILE * outputFile){
    long sourceChip = coreToRank(sourceCore);
    long destChip = coreToRank(destCore);
    char * isend = generateIsend(sourceChip,destChip,twTimeSend);
    char * ircv = generateIrecv(destChip, sourceChip, twTimeSend);

    fprintf(outputFile, isend);
    fprintf(outputFile, ircv);

    free(isend);
    free(ircv);

}

/**@}*/