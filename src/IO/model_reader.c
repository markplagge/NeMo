//
// Created by Mark Plagge on 4/28/17.
//

#include "IOStack.h"
#include <search.h>
#include "../lib/simclist.h"
#include "../neuro/tn_neuron.h"
#include "../lib/csv.h"
#include "../lib/lua.h"
#include "../lib/lualib.h"
#include "../lib/lauxlib.h"


//enum TNReadMode{
//    CONN , //Syn. Connectivity
//    AXTP, //Axon Types
//    SGI, //sigma GI vals
//    SP, //S Vals
//    BV, //b vals
//    NEXT, //goto next array data chunk
//    OUT //out of array data
//};


static enum modelReadMode fileReadState = START_READ;
//static enum TNReadMode tnReadState = CONN;


int countLines(FILE *fileHandle){
    int ch;
    int charsOnCurrentLine = 0;
    int count = 0;
    while ((ch = fgetc(fileHandle)) != EOF) {
        if (ch == '\n') {
            count++;
            charsOnCurrentLine = 0;
        } else {
            charsOnCurrentLine++;
        }
    }
    if (charsOnCurrentLine > 0) {
        count++;
    }
    return count;
}

void fldUpdate(int fldNum, enum modelReadMode rm){
    switch (rm) {

        case N_CONNECTIVITY:
        case N_AXONTYPES:
            if (fldNum % AXONS_IN_CORE == 0 ){
                rm ++;
            }
            break;
        case N_SGI:
        case N_BV:
        case N_SP:
            if (fldNum % NUM_NEURON_WEIGHTS == 0){
                rm ++;
            }
            break;
        case N_PARAMS:
            break;
        default:
            rm ++;
            break;
    }
    fldNum ++;
}

void tnFldRead(void *s, size_t len, void *data){
    struct  tnCSV * dat = (struct tnCSV *) data;
    enum modelReadMode rm = dat->rm;
    switch(rm){

    }
}

/**
 * loads the neurons from the model CSV into memory
 * @param fileHandle
 * @return the number of neurons extracted.
 */
int loadNeurons(FILE *fileHandle){
    int numNeurons = 0;
    char buffer[65535];
    struct csv_parser csvP;





}

void initModelInput(char *filename, int maxNeurons) {
    hcreate(maxNeurons);
    FILE * mdlf = fopen(filename, "rb");
    if (mdlf != NULL){
        char * itm = calloc(2048, sizeof(char));
        if (maxNeurons < 1){
            maxNeurons = countLines(mdlf);
            fclose(mdlf);
            mdlf = fopen(filename, "rb");
        }
        hcreate(maxNeurons);





    }
    else{

        tw_error(TW_LOC, "Model_Read","Error opening %s \n", filename);
    }
}


/**
 * Finds a neuron in the model config file. fills paramArray with the parameters from a file if
 * the neuron is in the config file. Returns -1 if neuron is not found, otherwise returns the number
 * of parameters found.
 * @param paramArray An initialized array of values, will be filled by the function.
 * @param core The neuron's core
 * @param local The neuron's local ID.
 * @return
 */
int getNeuronParameters(double *paramArray, int core, int local){

}