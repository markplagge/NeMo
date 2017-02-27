//
// Created by Mark Plagge on 2/21/17.
//

#include "input.h"
static readStatus netReadStat;
static fpos_t * CUR_NET_POS;
FILE* networkFile;
char * neuronFireFileName;
char * networkFileName;
char * spikeFileName;
FILE * spikeFile;
//menomization for file IO
static int minCore = 0;
static int maxCore = 0;
static int * coreLoc[];


/** Struct for managing full CSV read in */

struct csvFullDat{
    int fld_num;
    enum NeuronTypes type;
    int core_id;
    int local_id;
    int line_num;
};

/** Structs for managing neuron reads */
typedef struct CsvNeuron{
    int fld_num;
    int req_core_id;
    int req_local_id;
    char rawDat[32][1024];

}csvNeuron;

enum neuron_read_mode{
    START_READ,
    IN_ID,
    IN_DAT,
    OUT_ID,
    END_READ
};
static enum neuron_read_mode readMode = START_READ;


int openInputFiles(){
    networkFile = fopen(networkFileName,"r");
    if (errno){
        printf("Error opening network def "
                       "file %s, with error code %i.\n",networkFileName, errno);
        return errno;
    }
    netReadStat = 0;

    int ws = g_tw_npe;



    return 0;

}

int closeInputFiles(){
    fclose(networkFile);
    return 0;
}

/** Todo: Currently not used - API is strict for now, only neurons in network file. */
void forwardToNetwork(){
    char * linebuff = calloc(512, sizeof(char));
    int isEOL = 0;
    while (netReadStat == loaded){
        while(isEOL == 0){
            if(fgets(linebuff,512,networkFile) != NULL){

            }
            else{
                printf("Error encountered when reading network CSV - could not find a neuron def. \n");
            }
        }
    }
}

int* getCoresLinesInFile(){

}




/** Callback function called when libCSV has read an entire field. Add
 * new neuron types to this function. */

void fldRead(void *s, size_t len, void *data){
    //When a field is read, the data in string format is given here.
    //Used when first parsing the file
    struct csvFullDat * dat = (struct csvFullDat *)data;
    if(dat->fld_num  == 0){
        //Neuron Type Map Creation -- add new
        if ( strcmp( (char*) s,'TN') == 0){
                dat->type = TN;
        }

    } else if(dat->fld_num == 1){
        dat->core_id = atoi(s);
    } else if(dat->fld_num == 2){
        dat->local_id = atoi(s);
    } else{
        tw_lpid nGID = getGIDFromLocalIDs(dat->core_id,dat->local_id);
        neuronMap[nGID] = TN;
    }
    dat->fld_num ++;


}
void lineRead(int c, void *data) {
    //once a line has been read in, look at the global struct where data is stored.
    struct csvFullDat * dat = (struct csvFullDat *)data;
    if (c == -1){
        maxCore = dat->core_id;
    }else{
        if (minCore > dat->core_id){
            minCore = dat->core_id;
        }
    }
    dat->line_num ++;

}


/**
 * runs on mpi rank 0. Reads CSV file containing neurons,
 * then sets the LP type map for this simulation's neurons.
 * Also sets up the min/max/coremap values for use in readNeuron(),
 * so in-function menomization is not used.
 */
void parseNetworkFile(){
    //@TODO: Free this memory once TN_INIT has been called
    neuronMap = calloc(CORES_IN_SIM * NEURONS_IN_CORE,sizeof(int));
    struct csv_parser p;
    struct csvFullDat data = {0,0,0,0,0};

    char buf[4096];
    size_t bytes_read;
    if(csv_init(&p, CSV_APPEND_NULL) != 0) exit(EXIT_FAILURE);
    networkFile = fopen(networkFileName, "rb");

    while((bytes_read = fread(buf, 1, 4096, networkFile)) > 0){
        if(csv_parse(&p, buf, bytes_read, fldRead, lineRead, &data)){
            fprintf(stderr, "CSV Network Read Error\n");
            exit(EXIT_FAILURE);
        }
    }
    csv_free(&p) //, fldRead, lineRead, &data);

}

void postParseCleanup(){
    free(neuronMap);
    rewind(networkFile);
}
void neuron_fld(void *s, size_t len, void*data){
    csvNeuron *dat = (csvNeuron *) data;
    switch (dat->fld_num){
        case 0:
            break;
        case 1:
            dat->req_core_id = atoi((char*) s);
            readMode = IN_ID;
            break;
        case 2:
            dat->req_local_id = atoi((char*) s);
            readMode = OUT_ID;
            break;
        default:
            readMode = IN_DAT;
            dat->rawDat[dat->fld_num] = (char*) s;
    }
    dat->fld_num ++;

}
void neuron_line(int c, void *data) {
 readMode = END_READ;
}

/** Neuron init function -- currently hand-made TrueNorth init function -- this can be made more elegant, but
 * I want something that works right now.
 */
void initNeuron(tn_neuron_state *neuron, struct CsvNeuron csvN){
    for (int i = 0; i < 12; i ++){
        
    }
}
/** readNeuron - Currently, NeMo's neuron input system uses
 * libcsv. When requesting a neuron's config file (the calling function)
 * this function will go through the CSV file until either
 * the neuron Core:ID is found, or it hits EOF/end of search.
 * If a neuron is not defined in the API, then this function will
 * not modify the neuron state.
 *
 * @param core
 * @param nid
 * @param ntype
 * @param neuron
 */
void readNeuron(id_type core, id_type nid, int ntype, void* neuron){
    bool foundNeuron = false;
    readMode = START_READ;
    struct csv_parser csvP;
    struct CsvNeuron data = {0};
    char buf[4096];
    size_t bytes_read;
    if(csv_init(&csvP,CSV_APPEND_NULL) !=0) exit(EXIT_FAILURE);
    while((fgets(buf,  4096,networkFile))){
        if(csv_parse(&csvP, buf, bytes_read, neuron_fld, neuron_line, &data)){
            tw_error(TW_LOC,"CSV Neuron File Read Error\n");
        }
        switch (readMode){
            case IN_ID:
                break;
            case OUT_ID:
                //did we get the right core?
                if (core != data.req_core_id){
                    if (nid != data.req_local_id){
                        //wrong neuron, skip the line:
                        //If we weren't loading line by line this would be useful.
                    } else{
                        foundNeuron = true;
                    }
                }
                break;
            case END_READ:
                if(foundNeuron){
                    struct CsvNeuron * retDat = tw_calloc(TW_LOC,"IO", sizeof(retDat),1);
                    retDat->rawDat = data.rawDat;

                }

        }


    }



}