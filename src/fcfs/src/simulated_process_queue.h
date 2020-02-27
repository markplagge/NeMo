//
// Created by Mark Plagge on 2/26/20.
//





#ifndef SUPERNEMO_SIMULATED_PROCESS_QUEUE_H
#define SUPERNEMO_SIMULATED_PROCESS_QUEUE_H
#include "simulated_process.h"
#include <simclist.h>
#include <stdlib.h>

struct ProcQNode {
    struct ProcQNode *next;
    simulated_process *proc;
};
typedef struct ProcQueue{
    struct ProcQNode *front, *rear;
} proc_queue;

proc_queue *create_queue();
void proc_q_enqueue(proc_queue *q, simulated_process *p);
void proc_q_tick(proc_queue *q);
unsigned int proc_q_size(proc_queue *q);
int proc_q_top_cores(proc_queue *q);
simulated_process *proc_q_dequeue(proc_queue *q);

struct ProcQNode *create_new_node(simulated_process *p);





#endif //SUPERNEMO_SIMULATED_PROCESS_QUEUE_H
