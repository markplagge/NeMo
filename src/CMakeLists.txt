##cmake_minimum_required(VERSION 2.6)
project(NeMo)
enable_testing()

INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR} ${lmdir})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/lib/simclist)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(NON11 0 CACHE BOOL "Non C11 Compliant Compiler?")
set(USE_MODEL_RDR 1 CACHE BOOL "Use NeMo JSON Model reader lib?")

if (USE_LUA_JIT)
    message(AUTHOR_WARNING "USE JIT: ${USE_LUA_JIT}")

    ExternalProject_Add(project_luajit
            URL http://luajit.org/download/LuaJIT-2.1.0-beta2.tar.gz
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/luajit
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${jbld}
            BUILD_IN_SOURCE 1
            INSTALL_COMMAND make install
            PREFIX=${CMAKE_CURRENT_BINARY_DIR}/luajit
            )
    ExternalProject_Get_Property(project_luajit install_dir)
    add_library(luajit STATIC IMPORTED)
    set_property(TARGET luajit PROPERTY IMPORTED_LOCATION ${install_dir}/lib/libluajit-5.1.a)
    add_dependencies(luajit project_luajit)
    include_directories(${install_dir}/include/luajit-2.1)
    message(AUTHOR_WARNING " LJ ${install_dir}/include/luajit-2.1}")
else ()

    include_directories(
            ${LuaD}/src
            ${lua_BINARY_DIR})

endif ()

#SET(CMAKE_C_COMPILER mpicc)
#SET(CMAKE_CXX_COMPILER mpicxx)

##NeMo uses a few C11 features, including generic macros. We assume standard C11
##behaviors.
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lsqlite3")
if (${NON11})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

endif ()


## Model Version
LIST(APPEND CMAKE_MODULE_PATH "${ROSS_SOURCE_DIR}/cmake/")
#INCLUDE(GetGitRevisionDescription)
#GET_GIT_HEAD_REVISION(GIT_RESPEC_MODEL GIT_SHA1_MODEL)


## Model Compile-Time Options (Will add more later)
set(NeMo_NEURONS_IN_CORE 256 CACHE STRING "The number of neurons in each simulated core")
set(NeMo_NUM_WEIGHTS_IN_NEURON 4 CACHE STRING "How many weights does a neuron have? (4 is TN default)")
#set(NeMo_MODEL_FILE "./nemo_model.nfg1" CACHE STRING "Location and name of the NeMo Model Config File")
#set(NeMo_SPIKE_FILE "./nemo_spike.csv" CACHE STRING "Location and name of the NeMo input spike file")
set(NeMo_DEBUG 0 CACHE BOOL "Debug mode")
# set(NeMo_SAVE_MSG FALSE CACHE BOOL "Save message paths to CSV or Couch? (debug messages)")
# set(NeMo_SAVE_MSG_COUCH FALSE CACHE BOOL "Save message paths directly to couch?")
# set(NeMo_COUCH_ADD "None" CACHE STRING "The IP address:PORT of the couch database if used")
#


set(SAVE_MSGS 0)
set(USE_COUCH 0)
if (NeMo_SAVE_MSG)
    set(SAVE_MSGS 1)
    if (NeMo_SAVE_MSG_COUCH)
        set(USE_COUCH 1)
    endif ()
endif ()

## Build Configuration Header File
INCLUDE_DIRECTORIES(
        ${MPI_INCLUDE_PATH}
        ${CMAKE_CURRENT_BINARY_DIR})
#generated config files:
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/nemo_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/nemo_config.h @ONLY)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/nemo_config.h.in PROPERTIES GENERATED FALSE)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/nemo_config.h PROPERTIES GENERATED TRUE)


#Source Files:
set(model_srcs
        nemo_main.c
        globals.h
        globals.c
        neuro/synapse.h
        neuro/synapse.c

        neuro/axon.h
        neuro/axon.c

        neuro/tn_neuron.c
        neuro/tn_neuron.h

        tests/nemo_tests.h
        tests/synapse_test.c
        tests/bio_validation.c
        tests/mapping_tests.c
        tests/testIO.h
        tests/testIO.c


        mapping.c
        mapping.h


        lib/csv.h
        lib/libcsv.c


        IO/IOStack.c
        IO/IOStack.h
        IO/output.h
        IO/output.c
        nemo_config.h
        dumpi.c dumpi.h
        layer_map/layer_map_lib.h
        layer_map/layer_map_lib.c


        IO/neuron_output.c
        IO/model_reader.c
        IO/spike_db_reader.h
        IO/spike_db_reader.c
        neuro/tn_neuron_struct.h

        nemo_main.h tests/tomacs_exp.c tests/tomacs_exp.h lib/itrlve.h lib/itrlve.c lib/rqueue.c lib/rqueue.h lib/atomic_defs.h IO/model_reader_js.c)

#REMOVED:
#IO/spike_reader.c
#IO/spike_reader.h


ADD_EXECUTABLE(NeMo ${model_srcs})

#if (SQLITE_BUILD_STATIC)
#	set(SQLIB "${SQLITE_STATIC_NAME}")
#else()
#	set(SQLIB "${SQLITE_DYNAMIC_NAME}")
#endif()
set(ll ROSS sqlite3 simclist m)
if (USE_LUA_JIT)
    #TARGET_LINK_LIBRARIES(NeMo ROSS luajit sqlite3 simclist m)
    set(ll luajit ${ll})

else ()
    #TARGET_LINK_LIBRARIES(NeMo ROSS liblua sqlite3 simclist m)
    set(ll liblua ${ll})
endif ()
if(USE_MODEL_RDR)
    set(ll ModelReaderLib ${ll})
    message("lib settings: ${ll}")
endif()
message("-------------lib ${ll}")
TARGET_LINK_LIBRARIES(NeMo ${ll})
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_CURRENT_BINARY_DIR}/sqlite)
set(CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS} -L${CMAKE_CURRENT_BINARY_DIR}/sqlite)
set(CMAKE_STATIC_LINKER_FLAGS ${CMAKE_STATIC_LINKER_FLAGS} -L${CMAKE_CURRENT_BINARY_DIR}/sqlite)
set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} -L${CMAKE_CURRENT_BINARY_DIR}/sqlite)