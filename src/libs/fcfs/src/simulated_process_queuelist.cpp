//
// Created by Mark Plagge on 2/27/20.
//

#include "../include/fcfs_logic_system.h"
#include <simclist.h>
#include <stddef.h>
#include <stdlib.h>


int proc_q_list_enq(proc_q_list *q, simulated_process *p){
    int result = list_append(q->queue_list, p);
    return result;
}
simulated_process *proc_q_list_deq(proc_q_list *q){
    simulated_process *p = (simulated_process *)list_fetch(q->queue_list);
    list_delete_at(q->queue_list, 0);
    return p; //returns NULL on fail;

}

int proc_q_list_tick(proc_q_list *q){
    int erv = list_iterator_start(q->queue_list);
    if(erv){
        simulated_process *p;
        while ((p = (simulated_process *)list_iterator_next(q->queue_list))) {
            simulated_process_tick(p);
        }
        erv = list_iterator_stop(q->queue_list);
    }
    if(erv){
        erv = 0;
    }
    return erv;
}
int proc_q_list_top_needed_cores(proc_q_list *q){
    simulated_process *p = (simulated_process *)list_fetch(q->queue_list);
    if(p) {
        return p->needed_cores;
    }
    return -1;
}

proc_q_list *create_queue_list(){
    proc_q_list *q = (proc_q_list *) malloc(sizeof(proc_q_list));
    q->queue_list = (list_t *)malloc(sizeof(list_t));
    list_init(q->queue_list);
    return q;
}

int proc_q_list_size(proc_q_list *q){
    return list_size(q->queue_list);
}
double proc_q_list_get_next_start(proc_q_list *q){
    double res = -1;
    simulated_process *p = (simulated_process *)list_fetch(q->queue_list);


    if(p != NULL) {
        res = p->start_time;
    }
    return res;
}