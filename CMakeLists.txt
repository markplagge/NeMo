cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 99)
#ADD_COMPILE_OPTIONS(-std=gnu99)
#set(CMAKE_C_FLAGS "${CMAKE_C_CFLAGS} -std=gnu99")
project(SuperNeMo)

enable_testing()
include(ExternalProject)

set(CMAKE_C_FLAGS "${CMAKE_C_CFLAGS} -pthread ")

set (RD "${CMAKE_CURRENT_SOURCE_DIR}/ROSS/")
set (ND "${CMAKE_CURRENT_SOURCE_DIR}/src/")
set (LuaD "${CMAKE_CURRENT_SOURCE_DIR}/lua")
set (SQLD "${CMAKE_CURRENT_SOURCE_DIR}/sqlite/")
set (simcld "${CMAKE_CURRENT_SOURCE_DIR}/src/lib/simclist")

set(MLD_D "${CMAKE_CURRENT_SOURCE_DIR}/lib/model_reader")
#cmake_policy(SET CMP0012 OLD)
# LUA Options
#SET(USE_LUA_JIT FALSE BOOL)
option(USE_LUA_JIT "Use JIT lua project for scripting" OFF)
#sSET(USE_LUA_JIT TRUE)
ADD_SUBDIRECTORY(${RD})

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

