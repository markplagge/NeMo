project(ModelReader)
include(ExternalProject)
set(CMAKE_CXX_STANDARD 14)
set(reader_srcs
        main.cpp)
set(lib_srcs
        tn_parser.hh
        tn_parser.cpp
        lib/picojson.h lib/json_dto.hh)

set(RapidJSON_INCLUDE_DIRS lib)

#ADD_SUBDIRECTORY(../lib/rapidjson ${CMAKE_BINARY_DIR}/lib)
INCLUDE_DIRECTORIES(
        ${MPI_INCLUDE_PATH}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${ROSS_SOURCE_DIR}
        ${ROSS_BINARY_DIR})


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