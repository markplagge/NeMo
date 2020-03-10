//
// Created by Mark Plagge on 2/26/20.
//

#include <stdlib.h>
#include "simulated_process.h"


void simulated_process_tick(simulated_process *p){
    switch (p->current_state){
        case WAITING:
            ++ p->total_wait_time;
            break;
        case RUNNING:
            ++ p->total_run_time;
            if (p->total_run_time >= p->needed_run_time)
                p->current_state = COMPLETE;
            break;
        case COMPLETE:
            break;
    }
}

simulated_process *new_simulated_process_cores_time(int n_cores, int time_needed, int pid) {
    simulated_process *sim_proc = (simulated_process *) calloc(sizeof(struct SimulatedProcess), 1);
    sim_proc->needed_cores = n_cores;
    sim_proc->needed_run_time = time_needed;
    sim_proc->total_run_time = 0;
    sim_proc->total_wait_time = 0;
    sim_proc->current_state = WAITING;
    sim_proc->PID = pid;
    return sim_proc;
}




simulated_process *new_simulated_process_cores(int n_cores, int pid) {
    return new_simulated_process_cores_time(n_cores, pid, max_proc_time);
}
