//
// Created by Mark Plagge on 3/11/20.
//

#include "SimProcessQueue.h"
#include <sstream>
namespace neuro_os { namespace sim_proc{

    void SimProcessQueue::system_tick() {
        current_time ++;
        for ( auto &process : waiting_processes) {
            process.system_tick();
        }
        for ( auto &process : running_processes ) {
            process.system_tick();
        }
        for (auto it = pre_waiting_processes.begin();it != pre_waiting_processes.end();){
            if (it->getScheduledStartTime() >= current_time){
                waiting_processes.push_back(*it);
                pre_waiting_processes.erase(it);
            }
        }
    }


    int SimProcessQueue::get_next_process_size() {
        if (waiting_processes.size() > 0){
            return waiting_processes.front().getNeededCores();
        }else{
            return -1;
        }
    }


    void SimProcessQueue::start_next_process() {
        if(waiting_processes.size() > 0){
            auto p = waiting_processes.front();
            waiting_processes.pop_front();
            running_processes.push_back(p);
        }

    }


    long SimProcessQueue::get_total_process_wait_times() {
        long total_wait_time = 0;
        for (const auto &process : waiting_processes) {
            total_wait_time += process.getTotalWaitTime();
        }
        for (const auto &process : running_processes) {
            total_wait_time += process.getTotalWaitTime();
        }
        return total_wait_time;
    }

        nlohmann::json SimProcessQueue::to_json_obj() {
            using nlohmann::json;
            auto j = json({{"current_time",current_time},
                           {"waiting",waiting_processes},
                           {"running", running_processes},
                           {"pre_waiting", pre_waiting_processes}
                           });
            return j;

        }

        const std::deque<SimProcess<int>> &SimProcessQueue::getWaitingProcesses()  {
            return waiting_processes;
        }

        void SimProcessQueue::setWaitingProcesses(const std::deque<SimProcess<int>> &waitingProcesses) {
            waiting_processes = waitingProcesses;
        }

        const std::vector<SimProcess<int>> &SimProcessQueue::getRunningProcesses()  {
            return running_processes;
        }

        void SimProcessQueue::setRunningProcesses(const std::vector<SimProcess<int>> &runningProcesses) {
            running_processes = runningProcesses;
        }

        const std::deque<SimProcess<int>> &SimProcessQueue::getPreWaitingProcesses()  {
            return pre_waiting_processes;
        }

        void SimProcessQueue::setPreWaitingProcesses(const std::deque<SimProcess<int>> &preWaitingProcesses)  {
            pre_waiting_processes = preWaitingProcesses;
        }

        double SimProcessQueue::getCurrentTime() const  {
            return current_time;
        }

        void SimProcessQueue::setCurrentTime(double currentTime)  {
            current_time = currentTime;
        }

        void SimProcessQueue::from_json_obj(const json &j) {

        current_time = j.at("current_time").get<double>();
        if(j.find("waiting")!=j.end()) {
            waiting_processes = j.at("waiting").get<std::deque<SimProcess<int>>>();
        }
        if(j.find("running") != j.end()) {
            running_processes = j.at("running").get<std::vector<SimProcess<int>>>();
        }
        pre_waiting_processes = j.at("pre_waiting").get<std::deque<SimProcess<int>>>();

        }

        bool SimProcessQueue::operator==(const SimProcessQueue &rhs) const {
            return current_time == rhs.current_time &&
                    (waiting_processes == rhs.waiting_processes) &&
                    (running_processes == rhs.running_processes) &&
                    (pre_waiting_processes == rhs.pre_waiting_processes);
        }

        bool SimProcessQueue::operator!=(const SimProcessQueue &rhs) const {
            return !(rhs == *this);
        }


    }}