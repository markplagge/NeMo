cmake_minimum_required(VERSION 2.4)
project(NeMo)
enable_testing()

INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR})
set (CMAKE_EXPORT_COMPILE_COMMANDS 1 )

##NeMo uses a few C11 features, including generic macros. We assume standard C11
##behaviors.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

## Model Version
LIST(APPEND CMAKE_MODULE_PATH "${ROSS_SOURCE_DIR}/cmake/")
#INCLUDE(GetGitRevisionDescription)
#GET_GIT_HEAD_REVISION(GIT_RESPEC_MODEL GIT_SHA1_MODEL)




## Model Compile-Time Options (Will add more later)
set(NeMo_NEURONS_IN_CORE 256 CACHE STRING "The number of neurons in each simulated core")
set(NeMo_NUM_WEIGHTS_IN_NEURON 4 CACHE STRING "How many weights does a neuron have? (4 is TN default)")
set(NeMo_SAVE_MSG FALSE CACHE BOOL "Save message paths to CSV or Couch? (debug messages)")
set(NeMo_SAVE_MSG_COUCH FALSE CACHE BOOL "Save message paths directly to couch?")
set(NeMo_COUCH_ADD "None" CACHE STRING "The IP address:PORT of the couch database if used")

## Pras' data files
set(NeMo_WEIGHT_FILE "weights.csv" CACHE STRING "Pras' weight file")
set(NeMo_DATA_FILE "data.csv" CACHE STRING "Pras' data file")

###

set(SAVE_MSGS 0)
set(USE_COUCH 0)
if(NeMo_SAVE_MSG)
    set(SAVE_MSGS 1)
    if(NeMo_SAVE_MSG_COUCH)
        set(USE_COUCH 1)
    endif()
endif()

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

        tests/nemo_tests.h
        tests/synapse_test.c
        tests/bio_validation.c
        tests/mapping_tests.c



        mapping.c
        mapping.h


        IO/IOStack.c
        IO/IOStack.h

        IO/output.h
        IO/output.c
        nemo_config.h 
        nemo_main.h )


ADD_EXECUTABLE(NeMo  ${model_srcs})


TARGET_LINK_LIBRARIES(NeMo ROSS m)

