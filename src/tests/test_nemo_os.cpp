//
// Created by Mark Plagge on 3/14/20.
//

#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "../libs/neuro_os_config/include/neuro_os_config.h"

#include <ross.h>
#include "../nemo_os_system.h"

#include "../neuro/fcfs_core.hpp"
//#include "mapping.h"

extern "C" {
#include "../nemo_cpp_interfaces.h"
}
#include "../mapping.h"
#include "../neuro/axon.h"
#include "../neuro/synapse.h"
#include "../neuro/tn_neuron.h"
#include "../../libs/model_reader/include/tn_parser.hh"
#include "../nemo_os_globals.h"

TEST_CASE( "FCFS core test" ) {
    using namespace neuro_os;
    std::string fileloc;
    if (NEURO_OS_CONFIG_FILE_PATH[0] == '\0'){
        fileloc = std::string("../configs/fcfs_base.json");
    }else {
        fileloc = std::string(NEURO_OS_CONFIG_FILE_PATH);
    }

    std::ifstream i(fileloc.c_str());
    nlohmann::json j;
    i >> j;
    auto neuro_os_config = j.get<config::neuro_os_configuration>();
    auto cfg = config::neuro_os_configuration(neuro_os_config);
    GlobalConfig::get_instance(cfg);

    SECTION("FCFS Core Init") {

        FCFSCoreState *s = new FCFSCoreState();
        auto lp = new tw_lp();
        lp->gid = 0;
        lp->id = 0;
        fcfs_spike_config(s);
        for (auto &x : s->running_proc_size_encoders){
            REQUIRE(x.leak_rate == -1);
        }
        for (const auto &item : fileloc) {
            
        }

    }

}