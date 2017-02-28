//
// Created by Mark Plagge on 2/28/17.
//

#import "testIO.h"
#import <time.h>
int testInitInput(){
    int r = openInputFiles();

    oprint("Open Input File Result", r);
    return r;
}

int testCloseInput(){
    int r = closeNetworkFile();
    return r;
}

int testPreParseNetwork(){
    int r = 0;
    parseNetworkFile();

    for(int i = 0; i < NEURONS_IN_CORE * CORES_IN_SIM; i ++){
        r = neuronMap[i] && r;
    }

    return r;
}

int testNeuronRead(){
    int r = 0;
    //Performance test
    clock_t begin = clock();

    for(int core = 0; core < CORES_IN_SIM; core ++){
        for (int nid = 0; nid < NEURONS_IN_CORE; nid ++) {
            csvNeuron n = getNeuronData(core, nid);
            for (int i =0; i < MAX_NEURON_PARAMS; i ++ ){
                free(n.rawDat[i]);
            }
            free(n.rawDat);
        }


    }
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    csvNeuron n = getNeuronData(0,0);



    return r;
}