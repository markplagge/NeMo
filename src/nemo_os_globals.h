//
// Created by Mark Plagge on 3/12/20.
//

#ifndef SUPERNEMO_NEMO_OS_GLOBALS_H
#define SUPERNEMO_NEMO_OS_GLOBALS_H
#include "libs/neuro_os_config/include/neuro_os_config.h"
#include "singleton.h"
namespace neuro_os{


#include <iostream>
    class GlobalConfig : public Singleton<config::neuro_os_configuration>
    {
    private:
        GlobalConfig(config::neuro_os_configuration cg): g_config{cg}{}
    public:
        config::neuro_os_configuration g_config;

    };



    //inline std::unique_ptr<> global_neuro_config;
}
#endif //SUPERNEMO_NEMO_OS_GLOBALS_H
