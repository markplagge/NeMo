project(ModelReader)
include(ExternalProject)
set(CMAKE_CXX_STANDARD 14)
set(RJSON_PATH CACHE PATH "./extern/rapidjson" )
set(reader_srcs
        main.cpp tests/test_data.hh extern/CLI11.hh extern/rang.hh)
set(lib_srcs
        include/tn_parser.hh
        src/tn_parser.cpp
       src/utils.cpp src/utils.hh
        extern/json.hpp
        extern/tqdm.h
        include/model_reader_wrapper.h
        src/model_reader_wrapper.cc)

set(TN_READER_OMP 1 CACHE BOOL "Enable OMP?")

if(TN_READER_OMP)
    find_package(OpenMP)
        if (OPENMP_FOUND)
            message(STATUS "Found OpenMP")
            add_definitions(-DUSE_OMP=1)

            set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
            set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
            set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

        endif()
    message(STATUS "Did not find OpenMP - not enabled.")
endif()

## Testing Options:
set(TN_READER_TEST 1 CACHE BOOL "Enable testing of TN Reader?")
if(TN_READER_TEST)
    add_definitions(-DSRC_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")
        add_definitions(-DDEBUG)
        set(DEBUG 1)
        endif()

## Standards needed for rapidjson / model reader
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)
#ADD_SUBDIRECTORY(../libs/rapidjson ${CMAKE_BINARY_DIR}/libs)
INCLUDE_DIRECTORIES(
        ${MPI_INCLUDE_PATH}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${ROSS_SOURCE_DIR}
        ${ROSS_BINARY_DIR})



if(USE_SSE4_2)
    add_definitions(-DRAPIDJSON_SSE42)
        set(RAPIDJSON_USE_SSE42 TRUE)
        set(CMAKE_CXX"-march=native -mtune=native")
    message("RJSON using sse 4.2")
elseif(USE_SSE_2)
    add_definitions(-DRAPIDJSON_SSE2)
        set(RAPIDJSON_USE_SSE42 TRUE)
    message("RJSON using SSE 2")
else()
    message("No SIMD enabled for RJSON")
endif()

find_package(RapidJSON PATHS RJSON_PATH)
find_package(RapidJSON PATHS RJSON_PATH CONFIG)
#find_package(rapidjson)
if(RapidJSON_FOUND)
    message("Found RapidJSON through macro")
    message("RJSON custom flags: ${RapidJSON_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${RapidJSON_CXX_FLAGS})
else()
    set(RapidJSON_INCLUDE_DIRS extern)
    message("Did not find rapidjson - using internal build")
endif()

#message(STATUS "RJSON FLAGS: ${CMAKE_CXX_FLAGS}")

if(ARCH STREQUAL "ppc64le")
    add_definitions(-DRAPIDJSON_ENDIAN=1)
        SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++1y)
    message("Rapidjson is running in big endian mode.")
endif()

ADD_LIBRARY(ModelReaderLib ${lib_srcs})
ADD_EXECUTABLE(ModelReaderBin ${reader_srcs})

target_include_directories(ModelReaderLib PUBLIC
        ${RapidJSON_INCLUDE_DIRS}

        )
set_target_properties(ModelReaderBin
        PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

target_link_libraries(ModelReaderLib ROSS m)
target_link_libraries(ModelReaderBin ModelReaderLib)