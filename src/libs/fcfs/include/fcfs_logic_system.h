//
// Created by Mark Plagge on 2/26/20.
//





#ifndef SUPERNEMO_FCFS_LOGIC_SYSTEM_H
#define SUPERNEMO_FCFS_LOGIC_SYSTEM_H

#include <vector>
#include "../src/process_states.h"
#include "../src/SimProcess.h"
#include "../src/SimProcessSerial.h"
#include "../src/SimProcessQueue.h"
#define max_proc_time 64


namespace neuro_os {
    namespace sim_proc {


//simulated_processor defs

        template<class T>
        class SimProcess;

        class SimProcessQueue;

        class SimProcessSerial;
    }

}

#endif //SUPERNEMO_FCFS_LOGIC_SYSTEM_H