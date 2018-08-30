project(ModelReader)
include(ExternalProject)
set(CMAKE_CXX_STANDARD 14)
set(reader_srcs
        main.cpp tests/test_data.hh)
set(lib_srcs
        tn_parser.hh
        tn_parser.cpp
        extern/json_dto.hh utils.cpp utils.hh
        extern/json.hpp)

set(RapidJSON_INCLUDE_DIRS extern)
message(STATUS "RJSON FLAGS: ${CMAKE_CXX_FLAGS}")

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
        set(CMAKE_CXX"-march=native -mtune=native")
    message("RJSON using sse 4.2")
elseif(USE_SSE_2)
    add_definitions(-DRAPIDJSON_SSE2)
    message("RJSON using SSE 2")
else()
    message("No SIMD enabled for RJSON")
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