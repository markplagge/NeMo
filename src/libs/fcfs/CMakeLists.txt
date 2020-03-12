### FCFS PROCESS LIBRARY

OPTION(FCFS_ENABLE_UNIT_TESTS  "Enable FCFS unit tests (uses GoogleTest)" ON )
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/catch2/contrib ${CMAKE_MODULE_PATH})

set(FCFS_LIB_SRCS
        include/fcfs_logic_system.h
        lib/json.hpp
        src/SimProcess.cpp
        src/SimProcess.h
        src/process_states.h
        src/SimProcessSerial.cpp
        src/SimProcessSerial.h
        src/SimProcessQueue.cpp
        src/SimProcessQueue.h)

add_library(fcfs_logic_system  ${FCFS_LIB_SRCS})
#target_link_libraries(fcfs_logic_system simclist)
set_property(TARGET fcfs_logic_system PROPERTY CXX_STANDARD 14)
enable_testing()

set(FCFS_TEST_SRCS
        tests/test_test.cpp tests/test_sim_process_system.cpp tests/test_sim_process.cpp)
if(FCFS_ENABLE_UNIT_TESTS)
    set(CATCH_CONFIG_ENABLE_BENCHMARKING 1)
    add_compile_definitions(CATCH_CONFIG_ENABLE_BENCHMARKING)
    add_subdirectory(lib/catch2)
    set(CATCH_CONFIG_ENABLE_BENCHMARKING 1)
    add_executable(fcfs_tests ${FCFS_TEST_SRCS})
    target_link_libraries(fcfs_tests
            Catch2::Catch2)
    target_link_libraries(fcfs_tests  fcfs_logic_system)
    target_include_directories(fcfs_tests PUBLIC
            include)

    set_property(TARGET fcfs_tests PROPERTY CXX_STANDARD 14)
    include(CTest)
    include(ParseAndAddCatchTests)
    include(Catch)
    set(CATCH_BUILD_TESTING OFF)
    set(CATCH_INSTALL_DOCS OFF)
    set(CATCH_INSTALL_HELPERS ON)


    catch_discover_tests(fcfs_tests )
    ParseAndAddCatchTests(fcfs_tests)
    message("Enabled FCFS system tests")
endif()



