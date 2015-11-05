//
//  neuron_output.c
//  ROSS_TOP
//
//  Created by plaggm on 10/16/15.
//
//

#include "neuron_out_stats.h"


int write_csv_dyn(csvRow rows[], char* headers[], int numCols, int numRows, char const *fileName) {
	FILE *f = fopen(fileName, "w");
	if (f == NULL) return -1;
		//write headers of CSV File:
	for(int head = 0; head < numCols; head ++) {
		fprintf(f,"\"%s\",", headers[head]);
	}
	fprintf(f,"\n");

	for(int row = 0; row < numRows; row ++) {
			//row by row here
		csvRow *roww = &rows[row];
		do{
				//column
			fprintf(f, "\"%s\",",roww->data);
		}while((roww = roww->next) != NULL);
		fprintf(f, "\n");
	}
	fclose(f);
	return 0;
}


neuEvtLog *getLast(neuEvtLog *log) {
    if(log == NULL)
        return NULL;
    else if(log->next == NULL )
        return log;
    else
    {

         return getLast(log->next);
    }
}
int depthCounter(neuEvtLog *log){
    neuEvtLog *c = log;
    int counter = 0;
    do{
        counter ++;
        c = c->next;
    }while(c != NULL);
    return counter;
}
void addEntry(neuEvtLog *newE, neuEvtLog *log, int currentBigTick){
    //debugger:
    
    //int depth = depthCounter(log);
    
        
    //log->next = newE;
    newE->cbt = currentBigTick;
    //neuEvtLog *end = getLast(log); no longer putting the list of items at the end, putting in front.
    //end->next = newE;
    newE->next = log;
    log = newE;
    
    
}

int saveLog(neuEvtLog* log, char* fileName) {
    FILE * outfile;

    outfile = fopen(fileName, "a");
    //can't open file.
    if(outfile == NULL){
        return -2;
    }
    neuEvtLog *i = log;
    do{
        fprintf(outfile, "%Lf,%lu,%lu,%lu\n",i->timestamp,i->cid, i->nid, i->cbt);
        i = i->next;
    }while(i != NULL);
    
    fclose(outfile);
    
    return errno;
}

void testCSV() {
    int id1 = 0;
    int id2 = 1;
    int c1 = 0;
    int c2 = 0;
    
    long double ct;
    
    int bt = 0;
    
    
    int endBTS = 2000;
    
    neuEvtLog log1;
    neuEvtLog log2;
    log1.cbt = bt;
    log2.cbt = bt;
    log1.cid = c1;
    log2.cid = c2;
    log1.nid = id1;
    log2.nid = id2;
    ct = 0.0000222222222222222222222222222222;
    log1.timestamp = ct;

    log2.timestamp = ct;
    
    
    //non-pointer method
    for(int i = 1; i < endBTS; i ++)
    {
        
        neuEvtLog *nl1 = malloc(sizeof(neuEvtLog));
        neuEvtLog *nl2 =malloc(sizeof(neuEvtLog));
        nl1->cbt = i;
        nl2->cbt = i;
        nl1->cid = c1;
        nl2->cid = c2;
        nl1->nid = id1;
        nl2->nid = id2;
        
        ct ++;
        nl1->timestamp = ct;
        
        nl2->timestamp = ct;
        
        addEntry(nl1,&log1, i);
        addEntry(nl2,&log2, i);
        
        
    }
    
    printf("ran test.\n");
    
    saveLog(&log1, "log1.csv");
    saveLog(&log2, "log1.csv");
    
    
}
    
    
    
