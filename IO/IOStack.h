//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_IOSTACK_H
#define NEMO_IOSTACK_H

/**
 * { item_description }
 */
typedef struct CSV_Element {
	char * data;
	void * next;
}csvElement;

/**
 * { item_description }
 */
typedef struct CSV_Header {
	char * header;
	csvElement data;
}csvHeader;
/**
 * { item_description }
 */
typedef struct CSV_DATA{ 
	char * filename;
	char * metaFileName;
	char * metadata;

	csvHeader data[];

}csvData;

void initFiles();
void closeFiles();

void writeData(csvData *data);


#endif //NEMO_IOSTACK_H
