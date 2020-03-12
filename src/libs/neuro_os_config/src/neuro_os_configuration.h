//
// Created by Mark Plagge on 3/9/20.
//

#ifndef SUPERNEMO_NEURO_OS_CONFIGURATION_H
#define SUPERNEMO_NEURO_OS_CONFIGURATION_H
#include "../extern/json.hpp"
namespace neuro_os {
    namespace config {

        using nlohmann::json;

        class neuro_os_configuration {
        public:
            bool save_spike_events = false;
            unsigned int cores_in_chip = 4096;

            unsigned int chips_per_rank;
            unsigned int num_neuro_cores;
            std::string run_type;

            unsigned int num_chips_in_sim() const {
                return num_neuro_cores / cores_in_chip;
            }

            std::string stats_file;


        };

        void to_json(json &j, const neuro_os_configuration &c) {
            j = json{{"save_spike_events", c.save_spike_events},
                      {"cores_in_chip",     c.cores_in_chip},
                      {"num_neuro_cores",   c.num_neuro_cores},
                      {"chips_per_rank",    c.chips_per_rank},
                      {"stats_file",        c.stats_file},
                      {"run_type",          c.run_type}};

        }

        void from_json(const json &j, neuro_os_configuration &c) {
            j.at("save_spike_events").get_to(c.save_spike_events);
            j.at("num_neuro_cores").get_to(c.num_neuro_cores);
            j.at("stats_file").get_to(c.stats_file);
            j.at("cores_in_chip").get_to(c.cores_in_chip);
            j.at("chips_per_rank").get_to(c.chips_per_rank);
            j.at("run_type").get_to(c.run_type);
        }
    }

}
#endif //SUPERNEMO_NEURO_OS_CONFIGURATION_H
