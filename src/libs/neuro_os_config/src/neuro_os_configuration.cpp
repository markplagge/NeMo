//
// Created by Mark Plagge on 3/9/20.
//

#include <iostream>
#include "neuro_os_configuration.h"
namespace neuro_os {
    namespace config {
        using nlohmann::json;
        void to_json(json &j, const neuro_os_sim_proc_datafiles c) {
            j = json{{"spike_file", c.spike_file},
                     {"model",      c.model_path},
                     {"id",         c.id}};

        }

        void from_json(const json &j, neuro_os_sim_proc_datafiles c) {
            std::cout << j;
            j.at("spike_file").get_to(c.spike_file);
            j.at("model").get_to(c.model_path);
            j.at("id").get_to(c.id);
        }
        void to_json(json &j, const neuro_os_configuration &c) {
            j = json{{"save_spike_events", c.save_spike_events},
                     {"cores_in_chip",     c.cores_in_chip},
                     {"num_neuro_cores",   c.num_neuro_cores},
                     {"chips_per_rank",    c.chips_per_rank},
                     {"stats_file",        c.stats_file},
                     {"run_type",          c.run_type},
                     {"sim_procs",         c.sim_proc_files},
                     {"proc_file",          c.process_file}};

        }

        void from_json(const json &j, neuro_os_configuration &c) {
            j.at("save_spike_events").get_to(c.save_spike_events);
            j.at("num_neuro_cores").get_to(c.num_neuro_cores);
            j.at("stats_file").get_to(c.stats_file);
            j.at("cores_in_chip").get_to(c.cores_in_chip);
            j.at("chips_per_rank").get_to(c.chips_per_rank);
            j.at("run_type").get_to(c.run_type);
            j.at("proc_file").get_to(c.process_file);
            //j.at("sim_procs").get_to(c.sim_proc_files);
            for (const auto &item : j.at("sim_procs"))
            {
                auto f = item.get<neuro_os_sim_proc_datafiles>();
                f.id = item.at("id");
                f.model_path = item.at("model");
                f.spike_file = item.at("spike_file");
                c.sim_proc_files.push_back(f);
            }

            j.at("proc_file").get_to(c.process_file);
        }
    }
}