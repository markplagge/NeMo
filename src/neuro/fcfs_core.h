//
// Created by Mark Plagge on 2/26/20.
//





#ifndef SUPERNEMO_FCFS_CORE_H
#define SUPERNEMO_FCFS_CORE_H
#include "../globals.h"
#include "tn_neuron_struct.h"
#include "../fcfs/simulated_process.h"
#include "../fcfs/simulated_process_queue.h"
#include <stdlib.h>
#include <rand-clcg4.h>



typedef struct FcfsCoreState{
    int num_running_cores;

    int current_running_size;

    /** How long to check on process Q? */
    int process_tick_time;

    tw_stime  last_active_time;
    proc_queue * waiting_q;
    proc_queue * running_q;
    proc_queue * done_q;




    tn_neuron_state *inner_core_array[];
}fcfs_core_state;



#endif //SUPERNEMO_FCFS_CORE_H
