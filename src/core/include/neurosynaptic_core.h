//
// Created by Mark Plagge on 2019-09-16.
//





#ifndef SUPERNEMO_NEUROSYNAPTIC_CORE_H
#define SUPERNEMO_NEUROSYNAPTIC_CORE_H
#include <ross.h>
#include <vector>
#include "../bitfield_event_status.h"
#include "nemo_message_types.h"

#define NEURONS_PER_CORE 256
#define NEURON_FANOUT 1


#define RNG_START(lp) auto rng_count = lp->rng->count
#define RNG_END(lp)  msg->random_call_count = (lp->rng->count - rng_count)




struct NeurosynapticCore{
    /**
     * output_mode - sets the spike output mode of this core.
     * Mode 0 is no output,
     * Mode 1 is output spikes only
     * Mode 2 is all spikes output
     */
    int output_mode = 2;
    /**
 * The last time that this core had activity. This refers to any  message.
 */

    long double last_active_time = 0;
    unsigned long current_neuro_tick = 0;
    unsigned long previous_neuro_tick = -1;
    /**
 * The last time this core computed the leak.
 */
    unsigned long last_leak_time= 0 ;
    unsigned long leak_needed_count = 0;

    /**
     * A heartbeat check value.
     */
    bool heartbeat_sent = 0;
    /**
     * * the local core id. If linear mapping is enabled, then this will be equal to the GID/PE id
 */

    int core_local_id;
    /**
    * evt_stat holds the event status for the current event. This is used to compute
            * reverse computation. BF_Event_Stats is used instead of the tw_bf as it allows
    * more explicit naming. The concept is the same, however.
    */
    BF_Event_Status evt_stat;
    /* NeMo Neruo-Core Functionality */
    st


    /**
     * nemo_core_forward_heartbeat - Logic that manages
     * @param bf
     * @param cur_message
     * @param lp
     */
    void core_forward_heartbeat_handler(tw_bf *bf, nemo_message *cur_message, tw_lp *lp);
    void core_reverse_heartbeat_handler(tw_bf *bf, nemo_message *cur_message, tw_lp *lp);
    void core_send_heartbeat_handler(tw_bf *bf, nemo_message *cur_message,  tw_lp *lp);


};
/**@defgroup NCOREROSS
 * NeMo ROSS function headers */

/**
 * core initialization. Sets up the nemo_core contained in the *lp
 * @param lp
 */
void nemo_core_init(tw_lp *lp);
/**
 * pre-run functions for this nemo_core
 * @param lp
 */
void nemo_core_pre_run(tw_lp *lp);
/**
 * forward event handler for a nemo-core
 * @param bf
 * @param m
 * @param lp
 */
void nemo_core_forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp);
/**
 * reverse event handler for a nemo-core
 * @param bf
 * @param m
 * @param lp
 */
void nemo_core_reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp);
/**
 * commit function for a nemo-core
 * @param bf
 * @param m
 * @param lp
 */
void nemo_core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp);
/**
 * finalization of nemo-core
 * @param lp
 */
void nemo_core_finish(tw_lp *lp);

/** @}*/







#endif //SUPERNEMO_NEUROSYNAPTIC_CORE_H
