project(nero_os_config_subsystem)

set(neuro_os_src ./include/neuro_os_config.h ./src/neuro_os_config.cpp ./src/neuro_os_configuration.h ./src/neuro_os_configuration.cpp)
add_library(neuro_os_config ${neuro_os_src})