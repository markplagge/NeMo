#include "layer_map_lib.h"

#include "../globals.h"
#include "../mapping.h"

#include <stdio.h>

unsigned int CORES_PER_LAYER = 0;
unsigned int CHIPS_PER_LAYER = 0;


typedef struct ConInfo{
    id_type sourceNeuron;
    id_type destNeuron;
    id_type sourceCore;
    id_type destCore;
    id_type sourceChip;
    id_type destChip;
    id_type sourceLayer;
    id_type destLayer;
    tw_lpid sourceNeuronGID;


}conInfo;


void getGridLinearMap(conInfo * neuron){


    id_type sourceNeuronGlobal =  neuron->sourceNeuron + (NEURONS_IN_CORE * neuron->sourceCore);
    neuron->sourceNeuron = sourceNeuronGlobal;
    neuron->sourceChip = neuron->sourceCore / CORES_IN_CHIP;
    neuron->sourceLayer = neuron->sourceCore / CHIPS_PER_LAYER;
    neuron->sourceLayer = neuron->sourceChip / CHIPS_PER_LAYER;
    neuron->destLayer = neuron->sourceLayer + 1;
    neuron->destChip = ((neuron->destLayer * CHIPS_PER_LAYER)+
                        (neuron->sourceChip % CHIPS_PER_LAYER));
    neuron->destCore = ((neuron->destChip * CORES_IN_CHIP) +
                        (neuron->sourceCore % CORES_IN_CHIP));
    neuron->destNeuron = ((neuron->destCore * NEURONS_IN_CORE) +
                          (neuron->sourceNeuron % NEURONS_IN_CORE));

    id_type localSourceNeuron = getNeuronLocalFromGID(neuron->sourceNeuronGID);
    neuron->destNeuron = getGIDFromLocalIDs(neuron->destCore, localSourceNeuron);
    if(neuron->destLayer >= NUM_LAYERS_IN_SIM)
        neuron->destNeuron = 0;
    if(neuron->destNeuron >= SIM_SIZE){
        int x = 3;
//        tw_error(TW_LOC, "Invalid dest neuron. DEST gid was"
//                         "%lu, sim size is %lu. "
//                         "\n Neuron source GID was %lu."
//                         "Source N |\t Source Chip |\t SourceLayer |\t destLayer |\t destChip"
//                         " |\t destCore \n"
//                         "%llu\t|%llu\t|%llu\t|%llu\t|%llu\t|%llu\t|"
//                , neuron->destNeuron, SIM_SIZE, neuron->sourceNeuron, neuron->sourceChip,
//                neuron->sourceLayer, neuron->destLayer, neuron->destChip, neuron->destCore);
    }

}



tw_lpid getGridNeuronDest(unsigned int sourceCore, tw_lpid neuronGID) {
    conInfo * ncon = calloc(sizeof(conInfo),1);
    //ncon->sourceNeuron = sourceNeuron;
    ncon->sourceNeuron = getNeuronLocalFromGID(neuronGID);
    ncon->sourceCore = getCoreFromGID(neuronGID);
    ncon->sourceNeuronGID = neuronGID;
    if (LAYER_NET_MODE & OUTPUT_RND){
        if (LAYER_NET_MODE & OUTPUT_UNQ){
            tw_error(TW_LOC," UNIQUE NOT IMP");
            return -1; //Random unique grid mode
        }
        tw_error(TW_LOC," UNIQUE NOT IMP");
        return -1; //Random non-unique grid mode
    }
    //linear grid mode:
    /** @todo: optimize this */

    getGridLinearMap(ncon);
    if(ncon->sourceCore != sourceCore){
        tw_error(TW_LOC,"calculated source core != given source core.\n");
    }

    tw_lpid gidDest = ncon->destNeuron;//getGIDFromLocalIDs(ncon->destCore, ncon->destNeuron);
    free(ncon);
    return gidDest;
}


tw_lpid getNeuronDestInLayer(id_type sourceCore, tw_lpid neuronGID) {
    if(LAYER_NET_MODE & GRID_LAYER){
        return getGridNeuronDest(sourceCore, neuronGID);
    } else if (LAYER_NET_MODE & CONVOLUTIONAL_LAYER){
        return 0;
    }

}

void displayConfig(){
    char * rd = "Random ";

    char * ruq = "Unique Con";
    char * rnd = "Non-Unique Con";

    if(LAYER_NET_MODE != 0) {
        STT("Chips per layer: %i", CHIPS_PER_LAYER);
        STT("Cores per layer: %i", CORES_PER_LAYER);
        if (LAYER_NET_MODE & GRID_LAYER) {
            printf("* \t Layer Mode ");

        } else {
            printf("* \tCONV. Layer Mode \n");

        }
        printf("* ");
        if(LAYER_NET_MODE & OUTPUT_RND) {
            pm(rd);
            if (LAYER_NET_MODE & OUTPUT_UNQ) {
                pm(ruq);
            }else{
                pm(rnd)
            }

        }
        else {
            pm("Linear ");
        }
        STT("Layers in sim: %i", NUM_LAYERS_IN_SIM);
    }else{
        printf("* \tNot Layer Mode \n");
    }
}
/**
 * Setup initializes the layer network parameters
 */
void setupGrid(int showMapping){
    //initialize core params:
    if (GRID_ENABLE){
        //We are in a layer/grid network.
        switch(GRID_MODE){
            case 0:
                LAYER_NET_MODE = LAYER_NET_MODE | GRID_LAYER;
                break;
            case 1:
                LAYER_NET_MODE = LAYER_NET_MODE | CONVOLUTIONAL_LAYER;
                break;
        }
        if(RND_GRID)
            LAYER_NET_MODE = LAYER_NET_MODE | OUTPUT_RND;
        if(RND_UNIQ)
            LAYER_NET_MODE = LAYER_NET_MODE | OUTPUT_UNQ;
    }
    if(UNEVEN_LAYERS){
        tw_error(TW_LOC, "Sorry, uneven layers not supported yet.\n");

    }
    if (LAYER_NET_MODE & GRID_LAYER ) {

        //Grid layer - split cores/chips evenly across network.
        NUM_LAYERS_IN_SIM = NUM_CHIPS_IN_SIM / CHIPS_PER_LAYER;
        if(NUM_LAYERS_IN_SIM == 0){
            tw_error(TW_LOC, "Not enough layers defined");
        }
        CHIPS_PER_LAYER = NUM_CHIPS_IN_SIM /NUM_LAYERS_IN_SIM ;
        CORES_PER_LAYER = CORES_IN_SIM / NUM_LAYERS_IN_SIM;
        //if (NUM_LAYERS_IN_SIM % NUM_CHIPS_IN_SIM)
        //    tw_error(TW_LOC, "Non-even chip to layer distribution detected. Chose %ui chips and %ui layers", NUM_CHIPS_IN_SIM, NUM_LAYERS_IN_SIM);
    }
    


}
bool inFirstLayer(tn_neuron_state *s){
    if (s->myCoreID > CORES_PER_LAYER){
        return true;
    }
    return false;
};

bool inLastLayer(tn_neuron_state *s){
    if (s->myCoreID >= CORES_PER_LAYER * (NUM_LAYERS_IN_SIM - 1) || s->outputGID >= SIM_SIZE){
        return true;
    }
    return false;
}

void configureGridNeuron(tn_neuron_state *s, tw_lp *lp){
    for (int i = 0; i < NEURONS_IN_CORE; i ++){
        s->synapticConnectivity[i] = 1;
        s->axonTypes[i] = 0;
    }

    tw_lpid dest = getNeuronDestInLayer(s->myCoreID, lp->gid);
    if(inLastLayer(s)){
        s->outputGID = 0;
        s->posThreshold = 9999;
        s->lambda = 1000;
        s->epsilon = true;

    }else{
        s->outputGID = dest;
    }
    if(s->outputGID > (SIM_SIZE)){
        printf("sim size err.");
    }

}



void configureNeuronInLayer(tn_neuron_state *s, tw_lp *lp){
    switch(LAYER_NET_MODE){
        case GRID_LAYER:
            configureGridNeuron(s,lp);
            break;
        case CONVOLUTIONAL_LAYER:
            tw_error(TW_LOC, "Trying to init conv. layer network not implemented.");
            break;
        default:
            break;
    }
    if(inFirstLayer(s)){
        s->isSelfFiring = true;
        s->sigma_l = -1;
        s->epsilon = 0;
        s->lambda = 1;
    }
}
