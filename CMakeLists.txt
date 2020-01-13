cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

project(SuperNeMo LANGUAGES CXX C)

set (Project_Title "NeMo3")
set (Project_Desc " A parallel discrete event simulation based neuromorphic hardware simulation model / tool.")
set (Project_Author "Mark Plagge")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(CTest)
enable_testing()
include(ExternalProject)

set(ROSS_PROJECT "${CMAKE_CURRENT_SOURCE_DIR}/ROSS")

add_subdirectory(${ROSS_PROJECT})
add_subdirectory(src)
install(TARGETS NeMo DESTINATION bin)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )


#### Unit Tests ##
if(BUILD_TESTING)
    add_subdirectory(test)
endif(BUILD_TESTING)
