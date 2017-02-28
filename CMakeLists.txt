cmake_minimum_required(VERSION 2.6)
project(SuperNeMo)
enable_testing()

set (RD "${CMAKE_CURRENT_SOURCE_DIR}/ROSS/")
set (ND "${CMAKE_CURRENT_SOURCE_DIR}/src/")

ADD_SUBDIRECTORY(${RD})
ADD_SUBDIRECTORY(${ND})

SET(CMAKE_C_COMPILER mpicc)
SET(CMAKE_CXX_COMPILER mpicxx)
set_target_properties( NeMo
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
configure_file("demo.csv" "${CMAKE_BINARY_DIR}/bin/demo.csv" COPYONLY)
#TODO: Add targets to the build-tree export set
