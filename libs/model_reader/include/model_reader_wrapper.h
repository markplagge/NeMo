
//
// Created by Mark Plagge on 8/31/18.
//





#ifndef SUPERNEMO_MODEL_READER_WRAPPER_H
#define SUPERNEMO_MODEL_READER_WRAPPER_H
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifndef tn_neuron_state
    typedef struct TN_MODEL tn_neuron_state;
#endif
typedef struct TN_State_Wrapper TN_State_Wrapper;
typedef struct TN_Main TN_Main;

tn_neuron_state *get_neuron_state(unsigned long my_core, unsigned long neuron_id);
void init_neuron_state(unsigned long my_core, unsigned long neuron_id, tn_neuron_state *n);
tn_neuron_state *get_neuron_state_array(int my_core);
//TN_Main create_tn_data(char * filename);
void load_and_init_json_model(char *filename,int node);
int serial_load_json(char *json_filename);
void clean_up(TN_Main* m);
void create_neuron_model_mpi(char * filename, int my_rank,int num_ranks);
void clean_up_obj();
/** @todo: DEBUG CODE REMOVE WHEN DONE */
void debug_add_neuron_to_json(tn_neuron_state *s, tw_lp *lp);
 void debug_init_neuron_json();
 void debug_close_neuron_json();
 

#ifdef __cplusplus
}
#endif

#endif //SUPERNEMO_MODEL_READER_WRAPPER_H
