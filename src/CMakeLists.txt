project(NeMo LANGUAGES CXX)
enable_testing()

## Add MPI Libs
find_package(MPI)

INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR} ${lmdir})


set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(USE_MODEL_RDR 1 CACHE BOOL "Use NeMo->TN JSON Model reader lib?")
LIST(APPEND CMAKE_MODULE_PATH "${ROSS_SOURCE_DIR}/cmake/")


## Model Compile-Time Options (Will add more later)
set(NeMo_NEURONS_IN_CORE 256 CACHE STRING "The number of neurons in each simulated core")
set(NeMo_NUM_WEIGHTS_IN_NEURON 4 CACHE STRING "How many weights does a neuron have? (4 is TN default)")
set(NeMo_DEBUG 0 CACHE BOOL "Enable debug messages")


set(NEMO_NEURON_MODEL_SRCS
        core/neuron_models/lif_neuron.c
        core/neuron_models/lif_neuron.h)


set(NEMO_CORE_SRCS
        core/bitfield_event_status.h
        core/include/globals.h
        core/mapping.h
        core/include/nemo_core.h
        core/include/nemo_message_types.h
        core/include/neurosynaptic_core.h
        core/bitfield_event_status.cpp
        core/neurosynaptic_core.cpp
        core/mapping.cpp)
add_library(NeMoCore ${NEMO_CORE_SRCS} ${NEMO_NEURON_MODEL_SRCS})
target_link_libraries(NeMoCore PUBLIC ROSS MPI)
target_include_directories(NeMoCore PUBLIC ${ROSS_SOURCE_DIR} ${MPI_C_INCLUDE_PATH} ${MPI_CXX_INCLUDE_PATH})

### NeMo Executable
set(NEMO_SRC NeMo.cpp include/NeMo.h)

add_executable(NeMo ${NEMO_SRC})
target_include_directories(NeMo PUBLIC core/include ${ROSS_SOURCE_DIR} ${MPI_C_INCLUDE_PATH} )
target_link_libraries(NeMo PUBLIC ROSS MPI NeMoCore)

