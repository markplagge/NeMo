cmake_minimum_required(VERSION 2.4)
project(SuperNeMo)
enable_testing()

set (RD "${CMAKE_CURRENT_SOURCE_DIR}/ROSS/")
set (ND "${CMAKE_CURRENT_SOURCE_DIR}/src/")

ADD_SUBDIRECTORY(${RD})
ADD_SUBDIRECTORY(${ND})

set_target_properties( NeMo
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

#TODO: Add targets to the build-tree export set
