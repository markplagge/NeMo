cmake_minimum_required(VERSION 3.2)
project(nemo_tests)


include_directories(../src ../src/include ../src/core/include)
include_directories(../ROSS)
include_directories(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR})

set(TEST_SOURCES catch/catch.hpp NeMo_test.cpp)

add_executable(nemo_tests ${TEST_SOURCES})
target_include_directories(nemo_tests PUBLIC ${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR})
target_link_libraries(nemo_tests NeMoCore)
install(TARGETS nemo_tests DESTINATION bin)
