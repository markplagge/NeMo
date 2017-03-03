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
    enum neuronTypeVals type;
    int core_id;
    int local_id;
    int line_num;
};


enum neuron_read_mode{
    START_READ,
    IN_ID,
    IN_DAT,
    OUT_ID,
    END_READ
};
static enum neuron_read_mode readMode = START_READ;


int openInputFiles(){
    networkFile = fopen(networkFileName,"rb");
    if (errno){
        printf("Error opening network def "
                       "file %s, with error code %i.\n",networkFileName, errno);
        return errno;
    }
    netReadStat = 0;

    int ws = g_tw_npe;



    return 0;

}

int closeNetworkFile(){
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
        if ( strcmp( (char*) s,"TN") == 0){
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

char * netFile;



/**
 * runs on mpi rank 0. Reads CSV file containing neurons,
 * then sets the LP type map for this simulation's neurons.
 * Also sets up the min/max/coremap values for use in readNeuron(),
 * so in-function menomization is not used.
 */
void parseNetworkFile(){
    //@TODO: Free this memory once TN_INIT has been called
	//neuronMap = calloc(CORES_IN_SIM * NEURONS_IN_CORE,sizeof(enum neuronTypeVals));
	neuronMap = malloc(CORES_IN_SIM * NEURONS_IN_CORE * sizeof(enum neuronTypeVals));
	for (int i = 0; i < CORES_IN_SIM * NEURONS_IN_CORE; i ++){
		neuronMap[i] = NA;
	}
    struct csv_parser p;
    struct csvFullDat data = {0,0,0,0,0};

    char buf[4096];
    size_t bytes_read;
	size_t ttl_bytes = 0;
	
    if(csv_init(&p, CSV_APPEND_NULL) != 0) exit(EXIT_FAILURE);
    //networkFile = fopen(networkFileName, "rb");

    while((bytes_read = fread(buf, 1, 4096, networkFile)) > 0){
        if(csv_parse(&p, buf, bytes_read, fldRead, lineRead, &data) != bytes_read){
            fprintf(stderr, "CSV Network Read Error\n");
            exit(EXIT_FAILURE);
        }
		ttl_bytes += bytes_read;
    }
    csv_free(&p); //, fldRead, lineRead, &data);
	rewind(networkFile);
	//TODO: TEMP/debug hack for perf.
	netFile = malloc(ttl_bytes+1);
	//fgets(netFile,ttl_bytes,networkFile);
	fread(netFile, ttl_bytes, 1, networkFile);
	rewind(networkFile);

}

void postParseCleanup(){
    free(neuronMap);
    rewind(networkFile);
}

/** Function that checks if a neuron is in the file. Returns a line number to 
 start the search. Returns -1 if the neuron is not in the CSV file.*/
int findNeuronInFile(id_type core, id_type nid){
	tw_lpid nGID = getGIDFromLocalIDs(core,nid);
	int nType = neuronMap[nGID];
	if(nType){
		return 0;
	}
	else{
		return -1;
	}
	
}

/** CSV hanlder for flds read in - used for neuron parameter gathering. */
void neuron_fld(void *s, size_t len, void*data){
	//printf("Inside neuron fld.\n");
    csvNeuron *dat = (csvNeuron *) data;
	char * d = dat->rawDatM[dat->fld_num];
	char * csvD = (char*) s;
	
	
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
			
    }
	//dat->rawDat[dat->fld_num] = (char*) s;
	//sprintf(dat->rawDat[dat->fld_num],"%s",(char*) s);
	strcpy(d, csvD);
	//while( (*d++ = *csvD++) );
	
    dat->fld_num ++;

}
void neuron_line(int c, void *data) {
 readMode = END_READ;
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
struct CsvNeuron getNeuronData(id_type core, id_type nid) {
    bool foundNeuron = false;
    readMode = START_READ;
	rewind(networkFile);
    struct csv_parser csvP;
    struct CsvNeuron data;
    data.req_core_id=-1;
    data.req_local_id=-1;
    data.fld_num = 0;
	data.foundNeuron = 0;
	
	int startLN = findNeuronInFile(core, nid);
	if(startLN == -1){
		return data;
	}

	//    char ** neuronParams = tw_calloc(TW_LOC,"Neuron Read CSV", sizeof(char*), MAX_NEURON_PARAMS);
//	data.rawDat =  tw_calloc(TW_LOC,"Neuron Read CSV", sizeof(char*), MAX_NEURON_PARAMS);
//
//    for (int i =0; i < MAX_NEURON_PARAMS; i ++ ){
//        char * values = tw_calloc(TW_LOC,"Neuron Read CSV", sizeof(char), NEURON_BUFFER_SZ);
//		//neuronParams[i] = values;
//		data.rawDat[i] = values;
//    }

    char buf[65792];
    size_t bytes_read;
	
	
	if(csv_init(&csvP,CSV_APPEND_NULL) !=0) {
	exit(EXIT_FAILURE);
	}


	char cc[65792];
	//while((bytes_read = fread(buf, 1,  2048,networkFile)) > 0) {
	//	c = i;
	char * nf2 = netFile;
	
	while((*cc = *nf2++)){
		if(csv_parse(&csvP, cc, 1, neuron_fld, neuron_line, &data) != 1){
	//	while((bytes_read = fread(buf, sizeof(char), 1, networkFile)) > 0){
	//	if (csv_parse(&csvP, buf, bytes_read, neuron_fld, neuron_line, &data) != bytes_read) {
		
			tw_error(TW_LOC, "CSV Neuron File Read Error\n");
		//	}
		}
        switch (readMode) {
            case IN_ID:
			case START_READ:
                break;
            case OUT_ID:
                //did we get the right core?
				if (core == data.req_core_id && nid == data.req_local_id){
					foundNeuron = true;
				}
				
                break;
			case IN_DAT:
				//inside the data, nothing to do here...
				break;
            case END_READ:
                if (foundNeuron) {
                    data.foundNeuron = 1;
					csv_fini(&csvP, neuron_fld, neuron_line, &data);
					csv_free(&csvP);
                    return data;
                }
				
				readMode = START_READ;
				data.fld_num = 0;
				data.foundNeuron = 0;
				data.req_core_id = -1;
				data.req_local_id = -1;
                break;
			
				
        }
    }
    data.foundNeuron = 0;
	csv_fini(&csvP, neuron_fld, neuron_line, &data);
	csv_free(&csvP);
    return data;
}
