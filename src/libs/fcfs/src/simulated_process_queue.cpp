//
// Created by Mark Plagge on 2/26/20.
//

#include "simulated_process_queue.h"

struct ProcQNode *create_new_node(simulated_process *p){
    struct ProcQNode *new_node = (struct ProcQNode*) malloc(sizeof(struct ProcQNode));
    new_node->proc = p;
    new_node->next = NULL;
    return new_node;
}
proc_queue *create_queue(){
    proc_queue *q = (proc_queue*) malloc(sizeof(proc_queue));
    q->front = NULL;
    q->rear = NULL;
    return q;
}
void proc_q_enqueue(proc_queue *q, simulated_process *p){
    struct ProcQNode *new_node = create_new_node(p);
    if (q->rear == NULL){
        q->front = new_node;
        q->rear = new_node;
    }else{
        q->rear->next = new_node;
        q->rear = new_node;
    }
}

simulated_process *proc_q_dequeue(proc_queue *q){
    if (q->front == NULL){
        return NULL;
    }
    struct ProcQNode *temp = q->front;
    q->front = q->front->next;
    if (q->front == NULL){
        q->rear = NULL;
    }
    simulated_process *p = temp->proc;
    free(temp);
    return p;

}
void proc_node_tick(struct ProcQNode *n){
    if (n->proc != NULL){
        simulated_process_tick(n->proc);
    }
    if (n->next != NULL){
        proc_node_tick(n->next);
    }
}
void proc_q_tick(proc_queue *q){
    if(q->front != NULL){
        proc_node_tick(q->front);
    }
}
unsigned int proc_q_size(proc_queue *q){
    if (q->front == NULL){
        return 0;
    }
    int size = 0;
    struct ProcQNode *temp = q->front;
    while(temp != NULL){
        ++ size;
        temp = temp->next;
    }
    return size;
}

int proc_q_top_cores(proc_queue *q){
    if (q->front == NULL){
        return -1;
    }
    return q->front->proc->needed_cores;
}