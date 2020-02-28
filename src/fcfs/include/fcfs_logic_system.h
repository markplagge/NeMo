//
// Created by Mark Plagge on 2/26/20.
//





#ifndef SUPERNEMO_FCFS_LOGIC_SYSTEM_H
#define SUPERNEMO_FCFS_LOGIC_SYSTEM_H
#include <simclist.h>
#define max_proc_time 64
#ifdef __cplusplus
extern "C"
{
#endif
typedef enum  {
    WAITING,
    RUNNING,
    COMPLETE
}PROC_STATE;

typedef enum{
    CAN_ADD,
    IS_FULL
}SCHEDULER_STATE;

typedef struct SimulatedProcess{
    int PID;
    int needed_cores;

    int needed_run_time;
    int total_wait_time;
    int total_run_time;
    double start_time;
    PROC_STATE current_state;

}simulated_process;


typedef struct ProcQList{
    list_t *queue_list;
}proc_q_list;

//simulated_processor defs
simulated_process *new_simulated_process_cores_time(int n_cores, int time_needed,int pid);
simulated_process *new_simulated_process_cores(int n_cores, int pid);
void simulated_process_tick(simulated_process *p);



//processor queues
proc_q_list *create_queue_list();
int proc_q_list_top_needed_cores(proc_q_list *q);
int proc_q_list_tick(proc_q_list *q);
simulated_process *proc_q_list_deq(proc_q_list *q);
int proc_q_list_enq(proc_q_list *q, simulated_process *p);
int proc_q_list_size(proc_q_list *q);

double proc_q_list_get_next_start(proc_q_list *q);

#ifdef __cplusplus
}
#endif
#endif //SUPERNEMO_FCFS_LOGIC_SYSTEM_H