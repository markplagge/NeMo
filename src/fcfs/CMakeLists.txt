### FCFS PROCESS LIBRARY

OPTION(FCFS_ENABLE_UNIT_TESTS  "Enable FCFS unit tests (uses GoogleTest)" ON )


set(FCFS_SRCS
        include/tn_fcfs_features.h
        src/simulated_process.h
        src/simulated_process_queue.h
        src/simulated_process.c
        src/simulated_process_queue.c)



if(FCFS_ENABLE_UNIT_TESTS)

endif()

add_library(fcfs_simulated_process ${FCFS_SRCS})
