//
// Created by Mark Plagge on 5/25/16.
//

#include "IOStack.h"
#include <stdio.h>

// FILES:

FILE *messageFile;
FILE *neuronActivityFile;
FILE *neuronVoltageFile;


void printCSV(csv_data * head){
	
	csv_data * col = head;
	csv_data * row = head;
	printf("CSV DATA\n");
	while(row) {
		while (col){
			if (col->isString) {
				printf("\"%s\"", col->dataValue);
				if(col->nextCol){
					printf(",");
				}
			}else {
				printf("%s", col->dataValue);
				if(col->nextCol){
					printf(",");
				}
				
			}
			col = col->nextCol;
		}
		printf("\n");
		row = row->nextRow;
		col = row;
	}
	
}
/**
 * Creates an empty, ready to use csv_writer struct. Opens a file for writing as well.
 */
void initCSV(csv_writer * writer){
	
	//inside the csv_writer struct there is a data array and files.
	
	char filename[512];
	
	if(writer->isMPI) {
		if (writer->MPISepFiles) {
			sprintf(filename, "%s-%llu.csv", writer->fileName, writer->myRank);
		}
	}else{
		sprintf(filename, "%s.csv",writer->fileName);
	}
	
	writer->csvFile = fopen(filename,"w");
	
	writer->hdr = (csv_header *) calloc(sizeof(csv_header), 1);
	writer->data = (csv_data *) calloc(sizeof(csv_data), 1);
	writer->data->dataValue = 0;
	
}
void addCol(csv_writer *writer, char* data, int isTxt){
	
	csv_data * col=writer->data;
	if (!col->dataValue){
		col->dataValue = data;
		col->isString = isTxt;
		
	}else{
		while(col){
			if (col->nextCol) {
				col = col->nextCol;
			}else{
				col->nextCol = (csv_data *) calloc(sizeof(csv_data),1);
				col = col->nextCol;
				col->dataValue = data;
				col->isString = isTxt;
				col = 0;
			}
		}
	}
	
}
void writeElem(csv_data * curElem, csv_writer *writer){
	if(curElem){
		char *mod = curElem->isString?"\"":"";
		char *ed = curElem->nextCol?",":"\n";
		fprintf(writer->csvFile,"%s%s%s%s",mod,curElem->dataValue,mod,ed);
		if(curElem->nextCol){
			writeElem(curElem->nextCol,writer);
			
		}
		free(curElem);
		
	}
}
void writeHdr(csv_header *hdr, csv_writer* writer){
	while(hdr){
		char *ed = hdr->nextHeader?",":"\n";
		
		fprintf(writer->csvFile,"%s%s", hdr->header,ed);
		hdr = hdr->nextHeader;
	}
}
void writeRow(csv_writer * writer){
	writeElem(writer->data, writer);
	writer->data = (csv_data *) calloc(sizeof(csv_data), 1);
}

void closeCSV(csv_writer * writer){
	writeElem(writer->data,writer);
	fclose(writer->csvFile);
	free(writer);
}