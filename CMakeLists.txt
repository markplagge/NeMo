cmake_minimum_required(VERSION 3.5)
project(NeMo)
INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR})
set (CMAKE_EXPORT_COMPILE_COMMANDS 1 )
set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )
## Model Version
## Model Version
LIST(APPEND CMAKE_MODULE_PATH "${ROSS_SOURCE_DIR}/cmake/")
#INCLUDE(GetGitRevisionDescription)
#GET_GIT_HEAD_REVISION(GIT_RESPEC_MODEL GIT_SHA1_MODEL)

## Model Compile-Time Options (Will add more later)
set(NeMo_NEURONS_IN_CORE 512 CACHE STRING "The number of neurons in each simulated core")

## Build Configuration Header File
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
#generated config files:
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/nemo_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/nemo_config.h @ONLY)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/nemo_config.h.in PROPERTIES GENERATED FALSE)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/nemo_config.h PROPERTIES GENERATED TRUE)





set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


#Source Files:
set(model_srcs
        nemo_main.c
        globals.h
        globals.c
        neuro/synapse.h
        neuro/synapse.c

        neuro/axon.h
        neuro/axon.c

        neuro/tn_neuron.c
        neuro/tn_neuron.h

        mapping.c
        mapping.h
        bio_validation.c
        IO/IOStack.c
        IO/IOStack.h
        nemo_config.h 
        nemo_main.h)


ADD_EXECUTABLE(NeMo  ${model_srcs})


TARGET_LINK_LIBRARIES(NeMo ROSS m)