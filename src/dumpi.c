//
// Created by Mark Plagge on 8/18/17.
//

#include <printf.h>
#include "dumpi.h"
#include "ross.h"
#include "mapping.h"
#include <time.h>
/*
 * Set up datatypes for the MPI send/rcv cdoe
 */
int COUNT = 2; // One count for the COREid, one for the neuron local id.
int DTYPE = 11; // dtype  for the DUMPI file.
int COMM = 4;
int TAG = 0;
//unsigned int NUM_CHIPS_IN_SIM = CORES_IN_SIM / CORES_IN_CHIP;
//unsigned int CHIPS_PER_RANK; = g_tw_npe; //! Sets the number of chips per MPI rank for sim




const long double WALL_OFFSET = 0.0000002; //! Some sort of offset for the wall clock time - what is a good value?
const long double CPU_OFFSET = 0.0000002; //! The CPU time for recv. messages.
const long double NEURO_CORE_CLOCK = 1000; //! Neuromorphic core speed (cycles / second).
//new constansts for wall clock time
const long double JITTER_MAX 	= 0.0000009;
const long double JITTER_MIN 	= 0.000000001;
const long double COMPUTE_TIME  = 0.000002;
const long double SEND_TIME_MIN = 0.000005;
const long double SEND_TIME_MAX = 0.000050;

long double LAST_END_TIME_WC = 0;

long CURRENT_TICK = 0;
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

long double ld_rand( long double min, long double max )
{
	long double scale = rand() / (long double) RAND_MAX; /* [0, 1.0] */
	return min + scale * ( max - min );      /* [min, max] */
}

 long double jitter(){
	long double jit = ld_rand(JITTER_MIN, JITTER_MAX);
	 return jit;
}
 long double compTime(){
	//not truly random due to maths - but fast
	return COMPUTE_TIME + jitter();
}
int getCurrentTick(double now){
	return floorl(now);
}
long double sendTime(){
	return ld_rand(SEND_TIME_MIN, SEND_TIME_MAX);
}

/**@{ */
/**
 * getWallStart returns the wallStart param for dumpi
 * @param twSendTime
 * @return
 */

 long double getWallStart(long double twSendTime){

//	struct timespec start;
//	clock_gettime(CLOCK_REALTIME, &start);
//
//	double ctime = start.tv_sec / 100.0;
//	ctime = ctime + twSendTime;
//    return ctime;// + (arc4random() * WALL_OFFSET);
	int ct = getCurrentTick(twSendTime);
	if (ct != CURRENT_TICK){
		CURRENT_TICK = ct;
		LAST_END_TIME_WC = 0.0;
	}
	long double wctime = LAST_END_TIME_WC + compTime();
	return wctime;
}

/**
 * getWallEnd returns the  wallEnd param for dumpi.
 * @param startTime
 * @return
 */
long double getWallEnd(long double startTime){
	long double endTime = startTime + sendTime() + jitter();
	LAST_END_TIME_WC = endTime;
	return endTime;
}

/**
 * getCPUStart returns the cpuStart param for dumpi.
 * @param twSendTime
 * @return
 */
double getCPUStart(double twSendTime){
	return twSendTime;
	struct timespec start;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
	double ctime = start.tv_sec;
	ctime = ctime * CPU_OFFSET;
	return (( arc4random() % 1000) * CPU_OFFSET) + twSendTime;
	return ctime;
    //return (twSendTime * CPU_OFFSET ) + (arc4random() * CPU_OFFSET);
}

/**
 * getCPUEnd returns the cpuEnd param for dumpi.
 * @param twSendTime
 * @return
 */
double getCPUEnd(double cpuStart){
	//return cpuStart + CPU_OFFSET + (( arc4random() % 1000) * CPU_OFFSET);
	return cpuStart;
}

char * generateMsg(long sourceChip, long destChip, double twTimeSend,
                   char* type){
    char * outStr= calloc(sizeof(char), 512); // alloc new string - using this instead of buffer for the time being.
//    sourceChip = chipToRank(sourceChip);
//    destChip = chipToRank(destChip);
	size_type sc = chipToRank(sourceChip);
	size_type dt = chipToRank(destChip);
	long double wallStart = getWallStart(twTimeSend);
	long double wallEnd = getWallEnd(twTimeSend);
	long double cpuStart = getCPUStart(twTimeSend);
	long double cpuEnd = getCPUEnd(cpuStart);
	long t = sourceChip;
	if (type[5] == 'r'){
		sourceChip = destChip;
		destChip = t;
	}
    int rv = sprintf(outStr, "%li,%s %li %li %Lf %Lf %Lf %Lf %i %i %i %i\n",
                     t, type, sourceChip, destChip, wallStart,
                     wallEnd, cpuStart, cpuEnd, COUNT, DTYPE, COMM, TAG);

	//virtual ranks are interleaved - add a "running-on" feature for script extraction at the start of the line
	//rv = sprintf(outStr, "%li,%s",sourceChip,outStr);
    return outStr;
}
char * generateIsend(long sourceChip, long destChip, double twTimeSend){

    return generateMsg(sourceChip,destChip,twTimeSend,"MPI_Isend");
}
char * generateIrecv(long sourceChip, long destChip, double twTimeSend){
    return generateMsg(sourceChip, destChip, twTimeSend + (3 * CPU_OFFSET), "MPI_Irecv");
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