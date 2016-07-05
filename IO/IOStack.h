//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_IOSTACK_H
#define NEMO_IOSTACK_H
#include <stdio.h>
#include <stdlib.h>
#include "../globals.h"

#define addData(data, csv_writer) _Generic(data),\
messageData: addMessage,\
default: addDataStr)(data, csv_writer)


#define MEM_BUFF 128

typedef struct CSV_DATA{
	char * dataValue;
	int isString;
	int rowNum;
	void * nextCol;
	int valid;
	
}csv_data;

typedef struct CSV_HDRS{
	char * header;
	void * nextHeader;
}csv_header;

typedef struct CSV_WRITER{
	csv_header * hdr;
	csv_data * data;
	
	unsigned  short isMPI;
	unsigned short MPISepFiles;
	char * fileName;
	unsigned long myRank;
	
	int currentRow;
	FILE * csvFile;
} csv_writer;

extern csv_writer * messageTrace;

csv_writer *createCSV(char * filename, int myRank, int numRanks);
csv_writer *createCSVHDR(char * filename, int myRank, int numRanks, char * headers[], int numhdr);
void addCol(csv_writer *writer, char* data, int isTxt);
void writeRow(csv_writer * writer);
void addRow(csv_writer *writer);
void writeCSV(csv_writer *writer);
void closeCSV(csv_writer * writer);
void writeHdr(csv_header *hdr, csv_writer* writer);

/**
 * addMessage adds all of the message details to a writer object. */
void addMessage(messageData *message, csv_writer * csv_writer);
/**
 addDataStr saves what is assumed to be an array of strings to the csv file 
 @todo: not implemented yet, need to implement. */

void addDataStr(void * data, csv_writer * csv_writer);

typedef enum DATA_MODES{
	D_MPIO = 0x01,
	D_MSG_COUCH = 0x16,
	SV_MSG = 0x02,
	SV_VOLS = 0x04,
	SV_NEUACT = 0x08
	
}dataMode;

typedef struct IO_PARAMS{
	char * couchIP;
	
}io_params;

void initFiles(long numRanks, long myRank);
void closeFiles();




#endif //NEMO_IOSTACK_H
