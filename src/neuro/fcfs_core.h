//
// Created by Mark Plagge on 2/26/20.
//





#ifndef SUPERNEMO_FCFS_CORE_H
#define SUPERNEMO_FCFS_CORE_H
#include "../globals.h"
#include "tn_neuron_struct.h"
#include "simulated_process.h"

#include <stdlib.h>
#include <rand-clcg4.h>



typedef struct FcfsCoreState{
    int num_running_cores;
    int current_running_size;
    
    tn_neuron_state *inner_core_array[];
}fcfs_core_state;

#endif //SUPERNEMO_FCFS_CORE_H
