//
// Created by Mark Plagge on 2/26/20.
//





#ifndef SUPERNEMO_TN_FCFS_FEATURES_H
#define SUPERNEMO_TN_FCFS_FEATURES_H
#define max_proc_time 64

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
    PROC_STATE current_state;

}simulated_process;

struct ProcQNode {
    struct ProcQNode *next;
    simulated_process *proc;
};
typedef struct ProcQueue{
    struct ProcQNode *front, *rear;
} proc_queue;



simulated_process *new_simulated_process_cores_time(int n_cores, int time_needed,int pid);
simulated_process *new_simulated_process_cores(int n_cores, int pid);
proc_queue *create_queue();
void proc_q_enqueue(proc_queue *q, simulated_process *p);
simulated_process *proc_q_dequeue(proc_queue *q);
void proc_q_tick(proc_queue *q);
void simulated_process_tick(simulated_process *p);


#endif //SUPERNEMO_TN_FCFS_FEATURES_H
