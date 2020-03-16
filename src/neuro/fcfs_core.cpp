#include <climits>
//
// Created by Mark Plagge on 2/26/20.
//

#include <fstream>
#include <map>
#include "fcfs_core.hpp"
#include "tn_neuron.h"
#include "tn_neuron_struct.h"

#include "../nemo_os_globals.h"
std::vector<LIFBase> create_encoder(int id, int dest_id,int num_bits ){
    std::vector<LIFBase> lif_encoder_neurons;
    auto n = LIFBase();
    n.output_connectivity.push_back(dest_id);
    n.id = id;
    n.leak_rate = -1;
    for(int i = 0; i < num_bits; i ++){
        n.input_connectivity.push_back(1);
        n.weight_table.push_back(1);
    }
    lif_encoder_neurons.push_back(n);
    return lif_encoder_neurons;
    
}
std::vector<tn_neuron_state> create_adder(int id, int dest, int num_bits){
    std::vector<tn_neuron_state> adder;
    auto n = tn_neuron_state();
    n.axonTypes[0] = 0;
    n.axonTypes[1] = 0;
    n.weightSelection[0] = 0;
    n.synapticWeight[0] = 1;
    n.epsilon = 1;
    n.kappa = 1;
    n.resetMode = 1;
    n.isActiveNeuron = 1;
    n.posThreshold = 1;

    for(int i = 0; i < num_bits; i ++) {
        n.synapticConnectivity[i] = 1;
        n.axonTypes[i] = 0;
    }
    adder.push_back(n);
    return adder;
}


void fcfs_spike_config(FCFSCoreState *s){
    auto config = neuro_os::GlobalConfig::get_instance();
    //set up the encoding LIFs:
    std::vector<LIFBase> num_neuron_encoders = create_encoder(0,3,64);
    s->num_neuron_encoders = num_neuron_encoders;
    s-> waiting_proc_size_encoders = create_encoder(1,4,64);
    s-> running_proc_size_encoders = create_encoder(2,4,64);
    s->running_waiting_size_adder = create_adder(4, 5, 2);
    //64 bit encoder


    
    
    
    for(int i = 0; i < 64; i ++){
        auto v = LIFBase();
        
    }
    
    
    
}

//int start_next_proc(fcfs_core_state *s){
//    simulated_process *p = proc_q_dequeue(s->waiting_q);
//    if(p == NULL){
//        return -1;
//    }
//    proc_q_enqueue(s->running_q,p);
//    p->current_state = RUNNING;
//    return p->needed_cores;
//}
//
//int start_proc(fcfs_core_state *s, int t, int spike_input_x){
//    int start_proc_size = -1;
//
//    if (get_top_proc_size(s,t) > -1){
//        start_proc_size = start_next_proc(s);
//    }
//    return start_proc_size;
//}
//int epoch_check(fcfs_core_state *s, double timestep){
//    double epoc_diff = timestep - s->last_active_time;
//    return (int) epoc_diff; // would do floor, but this is just as effective.ß
//}
//void fcfs_core_tick(fcfs_core_state *s, int timestep){
//
//}
//
////spiking message handlers for the QUEUE
//spike_message *output_message(fcfs_core_state *s, double timestep);
//
//spike_message *input_message(fcfs_core_state *s, double timestep, spike_message *input_message);

// FCFS
void fcfs_core_init(FCFSCoreState *s, tw_lp *lp){
    std::cout << "INIT FCFS CORE \n";
    auto config = neuro_os::GlobalConfig::get_instance();
    fcfs_spike_config(s);

    //initialize processes
    neuro_os::sim_proc::SimProcessQueue q;
    std::ifstream i(config->process_file.c_str());
    nlohmann::json j;
    i >> j;
    q.from_json_obj(j);
    s->process_queue = q;
    for ( auto simProcFile : config->sim_proc_files) {
        s->pid_data_map[simProcFile.id] =  std::make_shared<neuro_os::config::neuro_os_sim_proc_datafiles>(simProcFile) ;
    }
    // Start sending fcfs update messages:
    tw_event *newEvent = tw_event_new(lp->gid,0.1,lp);
    auto *m = (messageData *) tw_event_data(newEvent);
    m->eventType = NEURO_OS_HEARTBEAT;
    
    tw_event_send(newEvent);
    


}
void fcfs_pre_run(FCFSCoreState *s, tw_lp *lp){

}
/** Initially, this is a non-neuron system - so we run the algorithm in a non-neuron fashion */
void fcfs_forward_event(FCFSCoreState *s, tw_bf *CV, messageData *m, tw_lp *lp ) {


}
void fcfs_reverse_event(FCFSCoreState *s, tw_bf *CV, messageData *m, tw_lp *lp ){

}
void fcfs_commit_event(FCFSCoreState *s, tw_bf *CV, messageData *m, tw_lp *lp){

}
void fcfs_final(FCFSCoreState *s, tw_lp *lp){

}


void LIFBase::integrate(double timestep, fcfs_communication *m, tw_lp *lp) {
    membrane_pot = membrane_pot + (weight_table[m->source_neuron] * input_connectivity[m->source_neuron]);
}

void LIFBase::leak(double timestep, fcfs_communication *m, tw_lp *lp) {
    while((++ last_active_time) <= timestep ){
        membrane_pot = membrane_pot + leak_rate;
    }
}

__unused bool LIFBase::fire(double timestep, fcfs_communication *m, tw_lp *lp) {
    if (membrane_pot > threshold){
        membrane_pot = reset_val;
        return true;
    }else{
        return false;
    }
}

void LIFBase::init_from_neuro_config(neuro_os::config::neuro_os_neuron_config *c) {
    
    membrane_pot = 0;
    weight_table = c->weights;
    input_connectivity = c->input_connectivity;
    output_connectivity = c->output_connections;
    leak_rate = c->leak_value;
}
int FCFSCoreState::encode_to_dest( int value, tw_lp *lp, unsigned int dest_neuron_id){
    for(int i = 0; i < value; i ++){
        auto evt = tw_event_new(dest_neuron_id, i, lp);
        messageData * m = (messageData *) tw_event_data(evt);
        m->eventType = NEURO_OS_STATE_CHANGE;
        m->axonID = dest_neuron_id;
        
        
    }
}



