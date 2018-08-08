project(ModelReader)

set(reader_srcs
        main.c
        tn_parser.hh
        tn_parser.cpp)
set(lib_srcs
        tn_parser.hh
        tn_parser.cpp)

ADD_EXECUTABLE(ModelReaderBin reader_srcs)
ADD_LIBRARY(ModelReaderLib lib_srcs)