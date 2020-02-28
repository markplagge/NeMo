//
// Created by Mark Plagge on 2/26/20.
//





#ifndef SUPERNEMO_FCFS_CORE_H
#define SUPERNEMO_FCFS_CORE_H
#include "../globals.h"
#include "tn_neuron_struct.h"
#include "../fcfs/include/fcfs_logic_system.h"
#include <stdlib.h>
constexpr int spike_ndims=2;
enum class FCFSMessageType{
    WAITING_PROCESS_NUM_CORES,
    RUNNING_PRROCESSES_SIZE,
    AVAIL_CORES,
    TOTAL_CORES,
    CAN_ADD_PROCESS,
    STANDARD_SPIKE

};
struct FCFSMessage{
    FCFSMessageType message_type;
    double spike_values[spike_ndims];
};


/** @defgroup FCFS FCFS Main
* FCFS Neurosynaptic Core - A custom spiking core implementing FCFS
 * Contains the primary forward / reverse functions for integration with NeMo
 * Similar to the SuperSynapse, but this core contains TN Neurons.
 *
 */

struct FcfsCoreState{
    int num_running_cores;
    int current_running_size;
    /** How long to check on process Q? */
    int process_tick_time;
    tw_stime  last_active_time;
    proc_q_list * waiting_q;
    proc_q_list * running_q;
    proc_q_list * done_q;
    tn_neuron_state *inner_core_array[];
    };
//
//
void fcfs_core_init(FcfsCoreState *s, tw_lp *lp);
void fcfs_forward_event(FcfsCoreState *s, tw_bf *CV, FCFSMessage *m, tw_lp *lp );
void fcfs_reverse_event(FcfsCoreState *s, tw_bf *CV, FCFSMessage *m, tw_lp *lp );
void fcfs_commit_event(FcfsCoreState *s, tw_bf *CV, FCFSMessage *m, tw_lp *lp);
void fcfs_final(FcfsCoreState *s, tw_lp *lp);
#endif //SUPERNEMO_FCFS_CORE_H
