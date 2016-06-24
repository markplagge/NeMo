//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_IOSTACK_H
#define NEMO_IOSTACK_H

typedef struct CSV_DATA{
	char * dataValue;
	int isString;
	
	void * nextCol;
	void * nextRow;
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
	
	FILE * csvFile;
} csv_writer;


void initCSV(csv_writer * writer);
void addCol(csv_writer *writer, char* data, int isTxt);
void writeRow(csv_writer * writer);
void closeCSV(csv_writer * writer);
void writeHdr(csv_header *hdr, csv_writer* writer);


typedef enum DATA_MODES{
	D_MPIO = 0x01,
	D_MSG_COUCH = 0x16,
	SV_MSG = 0x02,
	SV_VOLS = 0x04,
	SV_NEUACT = 0x08
	
}dataMode;

typedef struct IO_PARAMS{
	char * couchIP;
	
};

void initFiles(long numRanks, long myRank);
void closeFiles();

void writeData(csvData *data);


#endif //NEMO_IOSTACK_H
