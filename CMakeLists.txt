cmake_minimum_required(VERSION 2.6)
project(SuperNeMo)
enable_testing()

set (RD "${CMAKE_CURRENT_SOURCE_DIR}/ROSS/")
set (ND "${CMAKE_CURRENT_SOURCE_DIR}/src/")

ADD_SUBDIRECTORY(${RD})
ADD_SUBDIRECTORY(${ND})
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

SET(CMAKE_C_COMPILER mpicc)
SET(CMAKE_CXX_COMPILER mpicxx)
set_target_properties( NeMo
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        CONFIG_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/scripts"
)


#files for running IO:
#
file(COPY "./scripts/read_TN_json.py" DESTINATION ${CMAKE_BINARY_DIR}/scripts)
file(COPY "./scripts/api_def.py" DESTINATION ${CMAKE_BINARY_DIR}/scripts)
file(COPY "./scripts/read_TN_spikes.py" DESTINATION ${CMAKE_BINARY_DIR}/scripts)
configure_file("./NeMo_TN.py" "${CMAKE_BINARY_DIR}" COPYONLY)

configure_file("demo.csv" "${CMAKE_BINARY_DIR}/bin/demo.csv" COPYONLY)
#TODO: Add targets to the build-tree export set
