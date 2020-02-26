//
// Created by Mark Plagge on 2/26/20.
//





#ifndef SUPERNEMO_SIMULATED_PROCESS_H
#define SUPERNEMO_SIMULATED_PROCESS_H
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

simulated_process *new_simulated_process_cores_time(int n_cores, int time_needed,int pid);
simulated_process *new_simulated_process_cores(int n_cores, int pid);

void simulated_process_tick(simulated_process *p);



#endif //SUPERNEMO_SIMULATED_PROCESS_H
