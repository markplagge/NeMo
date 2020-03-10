//
// Created by Mark Plagge on 2/26/20.
//

#include "fcfs_core.hpp"




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

}
void fcfs_pre_run(FCFSCoreState *s, tw_lp *lp){

}
void fcfs_forward_event(FCFSCoreState *s, tw_bf *CV, FCFSMessage *m, tw_lp *lp ) {

}
void fcfs_reverse_event(FCFSCoreState *s, tw_bf *CV, FCFSMessage *m, tw_lp *lp ){

}
void fcfs_commit_event(FCFSCoreState *s, tw_bf *CV, FCFSMessage *m, tw_lp *lp){

}
void fcfs_final(FCFSCoreState *s, tw_lp *lp){

}