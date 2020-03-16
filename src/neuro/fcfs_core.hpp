//
// Created by Mark Plagge on 2/26/20.
//





#ifndef SUPERNEMO_FCFS_CORE_HPP
#define SUPERNEMO_FCFS_CORE_HPP
#include "../globals.h"
#include "tn_neuron_struct.h"

#include "../libs/fcfs/include/fcfs_logic_system.h"
#include "../libs/neuro_os_config/src/neuro_os_configuration.h"
#include <cstdlib>

constexpr int spike_ndims=2;




/** @defgroup FCFS FCFS Main
* FCFS Neurosynaptic Core - A custom spiking core implementing FCFS
 * Contains the primary forward / reverse functions for integration with NeMo
 * Similar to the SuperSynapse, but this core contains TN Neurons.
 *
 */

typedef enum {
    LIF_LOGIC,
    LIF_STATUS,
    LIF_STATE_CHANGE,
} LIF_Message_Meta;

struct LIFBase{
    double membrane_pot = 0;
    double threshold = 0;
    std::vector<double> weight_table;
    std::vector<bool> input_connectivity;
    std::vector<unsigned long> output_connectivity;

    int leak_rate = -1;
    double last_active_time = 0.0;
    double reset_val = 0;
    int id;
    

    void integrate(double timestep, fcfs_communication *m, tw_lp *lp);
    __unused void leak(double timestep,fcfs_communication *m, tw_lp *lp);

    __unused bool fire(double timestep,fcfs_communication *m, tw_lp *lp);
    void init_from_neuro_config(neuro_os::config::neuro_os_neuron_config *c);
    
};

struct FCFSCoreState{
    int num_running_cores;
    int current_running_size;
    /** How long to check on process Q? */
    int process_tick_time;
    tw_stime  last_active_time;
    neuro_os::sim_proc::SimProcessQueue process_queue;
    std::map<int,std::shared_ptr<neuro_os::config::neuro_os_sim_proc_datafiles>> pid_data_map;
    //tn_neuron_state inner_core_array[];
    std::vector<tn_neuron_state> inner_tn_core_array;
    
    std::vector<std::vector<tn_neuron_state>> neuron_ensembles;
    std::vector<std::vector<int>> connections;
    std::vector<LIFBase> num_neuron_encoders;
    std::vector<LIFBase> waiting_proc_size_encoders;
    std::vector<LIFBase> running_proc_size_encoders;
    std::vector<tn_neuron_state> running_waiting_size_adder;
    
    std::vector<LIFBase> inner_lif_core_array;
    std::vector<int> inner_lif_core_types;
    int encode_to_dest(int value, tw_lp *lp, unsigned int dest_neuron_id);
    };
//
//

void fcfs_core_init(FCFSCoreState *s, tw_lp *lp);
void fcfs_pre_run(FCFSCoreState *s, tw_lp *lp);
void fcfs_forward_event(FCFSCoreState *s, tw_bf *CV, messageData *m, tw_lp *lp );
void fcfs_reverse_event(FCFSCoreState *s, tw_bf *CV, messageData *m, tw_lp *lp );
void fcfs_commit_event(FCFSCoreState *s, tw_bf *CV, messageData *m, tw_lp *lp);
void fcfs_final(FCFSCoreState *s, tw_lp *lp);
void fcfs_spike_config(FCFSCoreState *s);
#endif //SUPERNEMO_FCFS_CORE_HPP
