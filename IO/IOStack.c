//
// Created by Mark Plagge on 5/25/16.
//

#include "IOStack.h"
#include <stdio.h>
#include <string.h>
// FILES:

FILE *messageFile;
FILE *neuronActivityFile;
FILE *neuronVoltageFile;


void writeFullRow(csv_data *row, csv_writer *writer, int currentRow){
	
	if(row && (currentRow == row->rowNum)){
		char *mod = row->isString?"\"":"";
		if(row->dataValue && row->nextCol && row->valid)
			fprintf(writer->csvFile, "%s%s%s",mod,row->dataValue,mod);
		if(row->nextCol){
			if (((csv_data *)row->nextCol)->rowNum == currentRow){
				fprintf(writer->csvFile, ",");
			}else{
				fprintf(writer->csvFile, "\n");
				++ currentRow;
			}
			if(row->nextCol){
				writeFullRow(row->nextCol, writer, currentRow);
			}
		}
		
	}
	if(row){
		//free(row->dataValue);
		free(row);
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

csv_writer *createCSV(char * filename, int myRank, int numRanks){
	/** @todo: switch this to tw_calloc */
	csv_writer *writer = (csv_writer *)calloc(sizeof(csv_writer), 1);
	if(numRanks){
		writer->isMPI = 1;
		writer->myRank = myRank;
	}
	writer->fileName = filename;
	
	initCSV(writer);
	
	return writer;
	
}
void writeCSV(csv_writer *writer){
	
	int currentRow = 0;
	//writeHdr(writer->hdr, writer);
	csv_data * row = writer->data;
	
	writeFullRow(row, writer, currentRow);
	fclose(writer->csvFile);
	initCSV(writer);
	
	
	
	//	while(row){
	//		while(col){
	//			csv_data * col2 = col->nextCol;
	//			writeElem(col, writer);
	//			//col = col->nextCol;
	//			col = col2;
	//		}
	//		fprintf(writer->csvFile,"\n");
	//		row = row->nextRow;
	//		col = row;
	//	}
	
}




void addRow(csv_writer *writer){
	writeCSV(writer);
	//if (writer->currentRow > MEM_BUFF ){
	//	writeCSV(writer);
	//	writer->currentRow = 0;
	//}else{
	//	++ writer->currentRow;
	//}
	
	
}
void addCol(csv_writer *writer, char* data, int isTxt){
	
	csv_data * col=writer->data;
	if (!col->dataValue){
		col->dataValue = data;
		col->isString = isTxt;
		col->rowNum = 0;
				col->valid = 1;
		
	}else{
		while(col){
			if (col->nextCol) {
				col = col->nextCol;
			}else{
				col->nextCol = (csv_data *) calloc(sizeof(csv_data),1);
				col = col->nextCol;
				col->dataValue = data;
				col->isString = isTxt;
				col->rowNum = writer->currentRow;
				col->valid = 1;
				col = 0;
			}
		}
	}
	
}
void writeHdr(csv_header *hdr, csv_writer* writer){
//	while(hdr){
//		csv_header * h2 = hdr->nextHeader;
//		char *ed = hdr->nextHeader?",":"\n";
//		
//		fprintf(writer->csvFile,"%s%s", hdr->header,ed);
//		free(hdr->header);
//		free(hdr);
//		hdr = h2;
//	}
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
void closeCSV(csv_writer * writer){
	writeElem(writer->data,writer);
	fclose(writer->csvFile);
	free(writer);
}
#ifdef SAVE_MSGS
void addMessage(messageData *message, csv_writer * csv_writer){
	char* data = (char *) calloc(sizeof(char),128);
	sprint(data,message->uuid);
	addCol(csv_writer, data,0);
	sprint(data,message->originGID);
	addCol(csv_writer, data,0);
	sprint(data,message->msgCreationTime);
	addCol(csv_writer, data,0);
	sprint(data,message->originComponent);
	addCol(csv_writer, data,1);
	addRow(csv_writer);

}
#endif

