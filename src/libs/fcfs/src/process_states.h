//
// Created by Mark Plagge on 3/11/20.
//

#ifndef SUPERNEMO_PROCESS_STATES_H
#define SUPERNEMO_PROCESS_STATES_H
namespace neuro_os {
    namespace sim_proc {
        typedef enum {
            WAITING,
            RUNNING,
            COMPLETE
        } PROC_STATE;

        typedef enum {
            CAN_ADD,
            IS_FULL
        } SCHEDULER_STATE;
    }
}
#endif //SUPERNEMO_PROCESS_STATES_H
