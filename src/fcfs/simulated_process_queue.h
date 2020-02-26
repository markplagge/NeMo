//
// Created by Mark Plagge on 2/26/20.
//





#ifndef SUPERNEMO_SIMULATED_PROCESS_QUEUE_H
#define SUPERNEMO_SIMULATED_PROCESS_QUEUE_H
#include "simulated_process.h"
#include <stdlib.h>
struct ProcQNode {
    struct ProcQNode *next;
    simulated_process *proc;
};
typedef struct ProcQueue{
    struct ProcQNode *front, *rear;
} proc_queue;

struct ProcQNode *create_new_node(simulated_process *p);
proc_queue *create_queue();
void proc_q_enqueue(proc_queue *q, simulated_process *p);
simulated_process *proc_q_dequeue(proc_queue *q);
void proc_q_tick(proc_queue *q);




#endif //SUPERNEMO_SIMULATED_PROCESS_QUEUE_H
