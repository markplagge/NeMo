#include "layer_map_lib.h"

#include "../globals.h"
#include "../mapping.h"

#include <stdio.h>

unsigned int CORES_PER_LAYER = 0;
unsigned int CHIPS_PER_LAYER = 0;

tw_lpid gridLinear(id_type sourceCoure, id_type sourceNeuron){
    //straight grid mapping
    //get the number of cores in each layer:
    //Calculate the destination core using the source core and cores per layer:
    id_type destCore = CORES_PER_LAYER + sourceCoure;
    //get destination GID using mylocalID and dest core:
    return getGIDFromLocalIDs(destCore, sourceNeuron);
}

tw_lpid getGridNeuronDest(unsigned int sourceCoure, unsigned int sourceNeuron){
    if (LAYER_NET_MODE & OUTPUT_RND){
        if (LAYER_NET_MODE & OUTPUT_UNQ){
            tw_error(TW_LOC," UNIQUE NOT IMP");
            return -1; //Random unique grid mode
        }
        tw_error(TW_LOC," UNIQUE NOT IMP");
        return -1; //Random non-unique grid mode
    }
    //linear grid mode:
    return gridLinear(sourceCoure,sourceNeuron);
}


tw_lpid getNeuronDestInLayer(unsigned int sourceCore, unsigned int sourceNeuron ){
    if(LAYER_NET_MODE & GRID_LAYER){
        return getGridNeuronDest(sourceCore, sourceNeuron);
    } else if(LAYER_NET_MODE & CONVOLUTIONAL_LAYER){
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
void configureGridNeuron(tn_neuron_state *s, tw_lp *lp){
    for (int i = 0; i < NEURONS_IN_CORE; i ++){
        s->synapticConnectivity[i] = 1;
        s->axonTypes[i] = 0;
    }
    tw_lpid dest = getNeuronDestInLayer(s->myCoreID, s->myLocalID);
    s->outputGID = dest;
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
