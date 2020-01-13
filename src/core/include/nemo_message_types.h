//
// Created by Mark Plagge on 2019-09-16.
//





#ifndef SUPERNEMO_NEMO_MESSAGE_TYPES_H
#define SUPERNEMO_NEMO_MESSAGE_TYPES_H


#include <string>
/**
 * message type enum
 */
enum nemo_message_type{
    NEURON_SPIKE,
    HEARTBEAT
};
/**
 * Main message data structure.
 *
 */

typedef struct Nemo_Message {
    int message_type;
    int source_core;
    int dest_axon;
    unsigned long intended_neuro_tick;
    uint32_t nemo_event_status;
    unsigned int random_call_count;
    double debug_time;
    std::string to_string();

}nemo_message;


#endif //SUPERNEMO_NEMO_MESSAGE_TYPES_H
