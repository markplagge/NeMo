### FCFS PROCESS LIBRARY

OPTION(FCFS_ENABLE_UNIT_TESTS  "Enable FCFS unit tests (uses GoogleTest)" ON )


set(FCFS_LIB_SRCS
        include/tn_fcfs_features.h
        src/simulated_process.h
        src/simulated_process_queue.h
        src/simulated_process.c
        src/simulated_process_queue.c )


set(FCFS_TEST_SRCS
        tests/test_test.c)
if(FCFS_ENABLE_UNIT_TESTS)
    find_package(Catch2 REQUIRED)
    add_executable(fcfs_tests ${FCFS_TEST_SRCS})

endif()

add_library(fcfs_simulated_process ${FCFS_LIB_SRCS})
