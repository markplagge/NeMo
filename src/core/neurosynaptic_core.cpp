//
// Created by Mark Plagge on 2019-09-16.
//

#include "include/neurosynaptic_core.h"

void NeurosynapticCore::core_forward_heartbeat_handler(tw_bf *bf, nemo_message *cur_message, tw_lp *lp) {
    if(!heartbeat_sent && cur_message->message_type == HEARTBEAT){
        tw_error(TW_LOC, "Got a heartbeat when no heartbeat was expected.\n");
    }
    auto heartbeat_rng = lp->rng->count;

    // Spikes are where heartbeats are generated. If no heartbeat has been sent this tick, and this is a spike,
    // then we need to send a heartbeat scheduled for the end of this current epoch.
    if(cur_message->message_type == NEURON_SPIKE){ // this if statement is a double check on the calling function
        if(heartbeat_sent && cur_message->intended_neuro_tick > current_neuro_tick){

            tw_error(TW_LOC, "Got a spike with an out of bounds tick.\n %s\n"
                             "Current core tick: %li\n"
                             "Current time: %Lf \n", cur_message->to_string().c_str(),this->current_neuro_tick,
                     tw_now(lp));
        }
        // Set event status to got spike:
        evt_stat = BF_Event_Status  :: Spike_Rec;
        /**
         * from tick 0->1:
         * current_neuro_tick = previous_neuro_tick = 0;
         * messages come in from t= 0.0...1 to t = 0.9;
         * if we get a spike and current_neuro_tick is < t:
         *  t = next_neuro_tick (gathered from the intended neuro tick in the message)
         * if no heartbeat is sent:
         *  heartbeat scheduled for t = 1
         * --
         * if message is a heartbeat:
         * if heartbeat intended neuro tick == current_neuro_tick:
         *  leak_needed_count = current_neuro_tick - last_leak_time
         *  previous_neuro_tick = current_neuro_tick
         *  last_leak_time = current_neuro_tick  <- this could be updated, but left in for possible different ways of calculating leak_needed_count
         *  do leak, reset, fire funs.
         *  fire messages are scheduled for current_neuro_tick + delay + JITTER (but this is handled by the implementation of this class)
         * else:
         *  This is an error condition.
         *
         */
        if(current_neuro_tick < cur_message->intended_neuro_tick){
            previous_neuro_tick = current_neuro_tick; //lossy operation - check for reverse computation errors
            bf->c0 = 1; //big tick change
            //this->evt_stat = BF_Event_Status ::NS_Tick_Update  & this->evt_stat; // evt stat update
            this->evt_stat = add_evt_status(this->evt_stat, BF_Event_Status::NS_Tick_Update);
            current_neuro_tick = cur_message -> intended_neuro_tick;

        } else if(current_neuro_tick == cur_message->intended_neuro_tick){
            bf->c0 = 0;
        }else{
            tw_error(TW_LOC, "Invalid tick times:\nMsg Data:\n %s \n current_neuro_tick: %d \n", cur_message->to_string().c_str(), this->current_neuro_tick);
        }
        if (!this->heartbeat_sent){
            bf->c1 = 1;
            this->evt_stat = add_evt_status(this->evt_stat,BF_Event_Status::Heartbeat_Sent);
            //this->evt_stat = BF_Event_Status::Heartbeat_Sent & this->evt_stat;
            this->heartbeat_sent = true;
            // send the heartbeat event
            this->core_send_heartbeat_handler(bf,cur_message,lp);

        }else{// some error conditions:
            if(cur_message->intended_neuro_tick != this->current_neuro_tick){
                tw_error(TW_LOC, "Got a spike intended for t %d, but heartbeat has been sent and LP is active at time %d.\n"
                                 "Details:\n"
                                 "CoreID: %i \n"
                                 "Message Data:\n"
                                 "source_core,dest_axon,intended_neuro_tick,nemo_event_status,"
                                 "random_call_count,debug_time %s\n", this->cur_message->intended_neuro_tick,
                         this->current_neuro_tick,this->core_local_id, cur_message->to_string().c_str());

            }
        }
    }else{
        evt_stat = BF_Event_Status ::Heartbeat_Rec;
        //error check:
        //message is heartbeat - We need to call leak, fire, reset logic
        //but first we set the heartbeat event to false.
        bf->c2 = 1;
        this->heartbeat_sent = false;
        //at this point, we are at neuro_tick t. We need to process the leak/reset/fire
        //functions for neurons. Leaks will loop for (previous_leak_time -> 0.).
        //however, this is the heartbeat management only function.
        //Also, the current neuro tick needs to be incremented.
        this->evt_stat = add_evt_status(this->evt_stat, BF_Event_Status::NS_Tick_Update);
        bf->c0 = 1;
        bf->c4 = 1; // C4 leak counts are updated.
        this->leak_needed_count = this->current_neuro_tick - this->last_leak_time;
        this->previous_neuro_tick = this->current_neuro_tick;
        // leak counter is ready. Last leak time needs to be updated as well.
        this->last_leak_time = this->previous_neuro_tick;
    }

}

void NeurosynapticCore::core_reverse_heartbeat_handler(tw_bf *bf, nemo_message *cur_message, tw_lp *lp) {

}

void NeurosynapticCore::core_send_heartbeat_handler(tw_bf *bf, nemo_message *cur_message, tw_lp *lp) {
    RNG_START(lp);

    auto now = tw_now(lp);
    //auto next_tick = get_next_neurosynaptic_tick(tw_now(my_lp));
    auto next_tick = 1.0;
    tw_event *heartbeat_event = tw_event_new(lp->gid,next_tick, my_lp);
    nemo_message *msg = (nemo_message *) tw_event_data(heartbeat_event);
    msg->intended_neuro_tick = next_tick;
    msg->message_type = HEARTBEAT;
    msg->debug_time = now;
    msg->nemo_event_status = as_integer(this->evt_stat);
    // Add some extra info to the messagE:
    msg->source_core = core_local_id;
    msg->dest_axon = -1;

    RNG_END(lp);

    tw_event_send(heartbeat_event);
}
