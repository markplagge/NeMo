//
// Created by Mark Plagge on 3/11/20.
//

#include "SimProcess.h"
#include <iostream>
namespace neuro_os { namespace sim_proc {


        template <class T>
        SimProcess<T> from_json_factory(const nlohmann::json &j ){
            auto pid = j.at("PID").get<int>();
            auto needed_cores = j.at("needed_cores").get<int>();
            auto needed_run_time = j.at("needed_run_time").get<int>();
            auto scheduled_start_time = j.at("scheduled_start_time").get<double>();
            T neuro_state = j.at("neuron_state_system").get<T>();
            auto p = SimProcess<T>(pid,needed_cores,needed_run_time,scheduled_start_time);
            return p;
        }



//        template <class T>
//        void from_json(const json &j, const SimProcess<T> &p){
//            j.at("PID").get_to(p.PID);
//            j.at("needed_cores").get_to(p.needed_cores);
//            j.at("needed_run_time").get_to(p.needed_run_time);
//            j.at("scheduled_start_time").get_to(p.scheduled_start_time);
//            j.at("neuron_state_system").get_to(p.neuron_state_system);
//        }

} }