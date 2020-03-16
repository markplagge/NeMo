//
// Created by Mark Plagge on 3/11/20.
//

#ifndef SUPERNEMO_SIMPROCESSQUEUE_H
#define SUPERNEMO_SIMPROCESSQUEUE_H
#include "../lib/json.hpp"
#include "SimProcess.h"
#include <deque>
#include <vector>
#include <string>
namespace neuro_os { namespace sim_proc {

    struct SimProcessQueue {
        std::deque<SimProcess<int>> waiting_processes;
        std::vector<SimProcess<int>> running_processes;
        std::deque<SimProcess<int>> pre_waiting_processes;
        double current_time = 0;

    public:
        void system_tick();
        int get_next_process_size();
        void start_next_process();
        long get_total_process_wait_times();
        nlohmann::json to_json_obj();
        void from_json_obj(const nlohmann::json &j);

        const std::deque<SimProcess<int>> &getWaitingProcesses() ;

        void setWaitingProcesses(const std::deque<SimProcess<int>> &waitingProcesses);

        const std::vector<SimProcess<int>> &getRunningProcesses() ;

        void setRunningProcesses(const std::vector<SimProcess<int>> &runningProcesses);

        const std::deque<SimProcess<int>> &getPreWaitingProcesses() ;

        void setPreWaitingProcesses(const std::deque<SimProcess<int>> &preWaitingProcesses) ;

        double getCurrentTime() const ;

        void setCurrentTime(double currentTime) ;

        void enqueue(SimProcess<int> &p){
            if(p.getScheduledStartTime() >= current_time){
                waiting_processes.push_back(p);
            }else{
                pre_waiting_processes.push_back(p);
            }
        }

        bool operator==(const SimProcessQueue &rhs) const;

        bool operator!=(const SimProcessQueue &rhs) const;

    };



} }


#endif //SUPERNEMO_SIMPROCESSQUEUE_H
