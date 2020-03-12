//
// Created by Mark Plagge on 3/12/20.
//

#ifndef SUPERNEMO_NEMO_CPP_INTERFACES_H
#define SUPERNEMO_NEMO_CPP_INTERFACES_H
#include <inttypes.h>
#include <stdbool.h>
struct nemo_global_struct{
    uint64_t LPS_PER_PE;
    uint64_t SIM_SIZE;
    uint64_t LP_PER_KP;

    bool IS_RAND_NETWORK;
    uint64_t CORES_IN_SIM;

//EXT size_type AXONS_IN_CORE;

    uint64_t CORE_SIZE;
    uint64_t SYNAPSES_IN_CORE;
    unsigned int NUM_LAYERS_IN_SIM;
    unsigned int LAYER_NET_MODE;
    unsigned int LAYER_SIZES[4096];
    unsigned int CORES_PER_LAYER;
    unsigned int CHIPS_PER_LAYER;
    unsigned int GRID_ENABLE;
    unsigned int GRID_MODE;
    unsigned int RND_GRID;
    unsigned int RND_UNIQ;
    unsigned int UNEVEN_LAYERS;
    unsigned int CORES_IN_CHIP;
    unsigned int NUM_CHIPS_IN_SIM;


};
void configure_from_nemo_os(struct nemo_global_struct * nemo_os_settings);
void displayModelSettings();
#endif //SUPERNEMO_NEMO_CPP_INTERFACES_H
