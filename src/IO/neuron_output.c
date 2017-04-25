//
// Created by Mark Plagge on 4/25/17.
//

#include "IOStack.h"
#include "../neuro/tn_neuron.h"
#include "ross.h"
#include "../mapping.h"
#include <string.h>


FILE * membraneFile;
FILE * outSpikeFile;
FILE * outNetworkCfgFile;
FILE * outNeuronCfgFile;

list_t networkStructureList;
list_t membranePotList;

int maxNeuronStruct = 512;
int maxNetworkListSize = 1024;
void openOutputFiles(char * outputFileName){
//    char ncfgfn[128] = {"\0"};
//    strcat(ncfgfn, "full_");
//    strcat(ncfgfn,outputFileName);
//    strcat(ncfgfn,".bin");

    outNetworkCfgFile = fopen(outputFileName,"wb");
//    outNeuronCfgFile = fopen(ncfgfn, "wb");

}
void closeOutputFiles(){
    fclose(outNetworkCfgFile);
}
void initDataStructures(int simSize){
    list_init(& networkStructureList);
    

}
/**
 * Writes neuron connections to a char array. Useful for making CSV files.
 * @param n
 * @param state
 */
int neuronConnToSCSV(tn_neuron_state *n, char *state){
    id_type destCore = getCoreFromGID(n->outputGID);
    id_type destAxon = getAxonLocal(n->outputGID);
    id_type core = n->myCoreID;
    id_type local = n->myLocalID;
    return sprintf(state,"%li,%li,%i,%i\n", core,local,destCore, destAxon);
}

void saveNetworkStructure(){

    openOutputFiles("network_def.csv");
    initDataStructures(g_tw_nlp);

    int neuronsStartAt = AXONS_IN_CORE + 1;
    fprintf(outNetworkCfgFile, "Core,NeuronID,DestCore,DestAxon\n");
    for (int core = 0; core < CORES_IN_SIM; core ++){
        for (int neuron = 0; neuron < NEURONS_IN_CORE; neuron ++){
            //save neuron connections to CSV file:
            tw_lpid ngid = getNeuronGlobal(core,neuron);
            tn_neuron_state *n = (tn_neuron_state * ) tw_getlocal_lp(ngid);

            char * line = tw_calloc(TW_LOC,"SAVE_STRUCTURE", sizeof(char), 512);
            neuronConnToSCSV(n, line);
            list_append(&networkStructureList, line);



        }
        if (list_size(&networkStructureList) > maxNetworkListSize){
            while(list_size(&networkStructureList) > 0){
                char * line = list_get_at(&networkStructureList, 0);
                fprintf(outNetworkCfgFile, line);
                list_delete_at(&networkStructureList, 0);
                free(line);
            }
        }
    }
    closeOutputFiles();


}




