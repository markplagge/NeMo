project(nero_os_config_subsystem LANGUAGES CXX)
set(CMAKE_Swift_LANGUAGE_VERSION  )

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(NOS_SRCS ./extern/json.hpp ./include/neuro_os_config.h ./src/neuro_os_config.cpp ./src/neuro_os_configuration.h ./src/neuro_os_configuration.cpp)
add_library(neuro_os_config ${NOS_SRCS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

