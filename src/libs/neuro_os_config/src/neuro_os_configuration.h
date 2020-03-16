//
// Created by Mark Plagge on 3/9/20.
//

#ifndef SUPERNEMO_NEURO_OS_CONFIGURATION_H
#define SUPERNEMO_NEURO_OS_CONFIGURATION_H
#include "../extern/json.hpp"
namespace neuro_os {
    namespace config {

        using nlohmann::json;
        struct neuro_os_sim_proc_datafiles{
            std::string model_path;
            std::string spike_file;
            int id;

        };
        struct neuro_os_neuron_config {
            int num_neurons;
            std::vector<double> weights;
            std::vector<bool> input_connectivity;
            std::vector<unsigned long> output_connections;
            int leak_value;
            double threshold;
            double reset_val;
                        
        };
        void to_json(json &j, const neuro_os_sim_proc_datafiles c);
        void from_json(const json &j, neuro_os_sim_proc_datafiles c);



        class neuro_os_configuration {
        public:
            bool save_spike_events = false;
            unsigned int cores_in_chip = 4096;
            unsigned int chips_per_rank;
            unsigned int num_neuro_cores;
            std::string run_type;
            std::vector<neuro_os_sim_proc_datafiles> sim_proc_files;
            unsigned int num_chips_in_sim() const {
                return num_neuro_cores / cores_in_chip;
            }
            std::string process_file;
            std::string stats_file;
            std::vector<neuro_os_neuron_config> scheduler_algorithm_neurons;




        };
        void to_json(json &j, const neuro_os_configuration &c);
        void from_json(const json &j, neuro_os_configuration &c);
    }

}
#endif //SUPERNEMO_NEURO_OS_CONFIGURATION_H
