cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)
#ADD_COMPILE_OPTIONS(-std=gnu99)
#set(CMAKE_C_FLAGS "${CMAKE_C_CFLAGS} -std=gnu99")
project(SuperNeMo)

set (Project_Title "NeMo")
set (Project_Desc " A parallel discrete event simulation based neuromorphic hardware simulation model / tool.")
set (Project_Author "Mark Plagge")


set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set (AUTO_MPI 0 CACHE BOOL "Automatically configure MPI libs/headers? ALternatively, you need to set the C compiler to
the location of mpicc, and the C++ compiler to mpicxx. ")
if(AUTO_MPI)
    find_package(MPI)
        add_library(mpi-l INTERFACE IMPORTED)
    set_property(TARGET mpi-l PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${MPI_C_INCLUDE_PATH})
    add_library(mpi SHARED IMPORTED)
    set_property(TARGET mpi PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${MPI_C_INCLUDE_PATH})
    set_property(TARGET mpi PROPERTY IMPORTED_LOCATION ${MPI_C_LIBRARIES})
    message("MPI Include Path: ${MPI_C_INCLUDE_PATH}")
endif(AUTO_MPI)



enable_testing()
include(ExternalProject)

#
#
## Set up some base commands:
message("ARCH:${ARCH}|")
set(arch_chk ${ARCH})
if (arch_chk STREQUAL "x86_64")
    include(OptimizeForArchitecture)
    message("Auto opt inabled")
    OptimizeForArchitecture()
    foreach(flag ${Vc_ARCHITECTURE_FLAGS})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    endforeach()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    message("Clang/GNU Compiler detected")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -mtune=native")
    set (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -march=native -mtune=native")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread ")


#Sub Project Directories
set (RD "${CMAKE_CURRENT_SOURCE_DIR}/ROSS/")
set (ND "${CMAKE_CURRENT_SOURCE_DIR}/src/")
set (LuaD "${CMAKE_CURRENT_SOURCE_DIR}/lua")
set (SQLD "${CMAKE_CURRENT_SOURCE_DIR}/sqlite/")
set (simcld "${CMAKE_CURRENT_SOURCE_DIR}/src/lib/simclist")
set(MLD_D "${CMAKE_CURRENT_SOURCE_DIR}/libs/model_reader")
#cmake_policy(SET CMP0012 OLD)
# LUA Options
#SET(USE_LUA_JIT FALSE BOOL)
option(USE_LUA_JIT "Use JIT lua project for scripting" OFF)
#sSET(USE_LUA_JIT TRUE)
ADD_SUBDIRECTORY(${RD})
if (arch_chk STREQUAL "x86_64")
    SET (TARGET ROSS PROPERTY CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
endif()
ADD_SUBDIRECTORY(${SQLD})
ADD_SUBDIRECTORY(${simcld})

if(USE_LUA_JIT)
    message(STATUS "LUA JIT PROJECT" )

else()
    ADD_SUBDIRECTORY(${LuaD})
    include_directories(${MPI_INCLUDE_PATH}
            ${LuaD}/src
            ${lua_BINARY_DIR})
endif()

ADD_SUBDIRECTORY(${ND})
add_subdirectory(${MLD_D})
#find_package(MPI REQUIRED)



#get_target_property(CFLGS ROSS CMAKE_C_FLAGS)
set_target_properties( NeMo
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CONFIG_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/scripts"
        USE_LUA_JIT ${USE_LUA_JIT}
 #       CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLGS} -std=gnu11"
)


#files for running IO:
#


configure_file("demo.csv" "${CMAKE_BINARY_DIR}/bin/demo.csv" COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/IO/model_read.lua ${CMAKE_CURRENT_BINARY_DIR}/bin/model_read.lua COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/IO/tn_types.lua ${CMAKE_CURRENT_BINARY_DIR}/bin/tn_types.lua COPYONLY)


#Model Reader Lib and Exe:

