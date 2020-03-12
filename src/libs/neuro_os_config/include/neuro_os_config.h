//
// Created by Mark Plagge on 3/9/20.
//

#ifndef SUPERNEMO_NEURO_OS_CONFIG_H
#define SUPERNEMO_NEURO_OS_CONFIG_H
#include "../src/neuro_os_configuration.h"
namespace neuro_os {
    namespace config {
        class neuro_os_configuration;
        std::unique_ptr<neuro_os_configuration> global_neuro_config;
    }
}
#endif //SUPERNEMO_NEURO_OS_CONFIG_H
