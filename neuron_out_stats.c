//
//  neuron_output.c
//  ROSS_TOP
//
//  Created by plaggm on 10/16/15.
//
//

#include "neuron_out_stats.h"



neuEvtLog *getLast(neuEvtLog *log) {
    if(log == NULL)
        return NULL;
    else if(log->next == NULL || log->next->timestamp || ! log->next)
        return log;
    else
    {

         getLast(log->next);
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
void addEntry(neuEvtLog *newE, neuEvtLog* log, int cbt){
    //debugger:
    
    int depth = depthCounter(log);
    
        
    //log->next = newE;
    newE->cbt = cbt;
    neuEvtLog *end = getLast(log);
    end->next = newE;
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
    ct = (uint16_t)random() / 1000;
    log1.timestamp = ct;
    ct = (uint16_t)random() / 1000;
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
        
        ct = (long double)i + (long double)((uint16_t)random()) /100000.0;
        nl1->timestamp = ct;
        ct = i + ((uint16_t)random()) /100000.0;
        nl2->timestamp = ct;
        
        addEntry(nl1,&log1, i);
        addEntry(nl2,&log2, i);
        
        
    }
    
    printf("ran test.\n");
    
    saveLog(&log1, "log1.csv");
    saveLog(&log2, "log1.csv");
    
    
}
    
    
    
