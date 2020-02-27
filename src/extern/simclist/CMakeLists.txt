cmake_minimum_required(VERSION 2.6)
PROJECT(simclist)

# simclist options
OPTION(SIMCLIST_DEBUG
    "Build with debug code and debug symbols enabled"
    OFF)
OPTION(SIMCLIST_THREADING
    "Build with simclist threading enable"
    OFF)
OPTION(SIMCLIST_NO_DUMPRESTORE
    "Disable building of dump & restore functionalities"
    OFF)
OPTION(SIMCLIST_ALLOW_LOCATIONBASED_HASHES
    "Allow list_hash() to work exclusively on memory locations"
    OFF)

# expand selected options
SET(SIMCCFLAGS "")
# build with debug?
IF(SIMCLIST_DEBUG)
    SET(SIMCCFLAGS "${SIMCCFLAGS} -DSIMCLIST_DEBUG")
ENDIF(SIMCLIST_DEBUG)
# build with threading?
IF(SIMCLIST_THREADING)
    SET(SIMCCFLAGS "${SIMCCFLAGS} -DSIMCLIST_WITH_THREADS")
ENDIF(SIMCLIST_THREADING)
# build without dump/restore functionalities?
IF(SIMCLIST_NO_DUMPRESTORE)
    SET(SIMCCFLAGS "${SIMCCFLAGS} -DSIMCLIST_NO_DUMPRESTORE")
ENDIF(SIMCLIST_NO_DUMPRESTORE)
IF(SIMCLIST_ALLOW_LOCATIONBASED_HASHES)
    SET(SIMCCFLAGS "${SIMCCFLAGS} -DSIMCLIST_ALLOW_LOCATIONBASED_HASHES")
ENDIF(SIMCLIST_ALLOW_LOCATIONBASED_HASHES)
SET_SOURCE_FILES_PROPERTIES(simclist.c COMPILE_FLAGS "${SIMCCFLAGS}")

# main building stuff
ADD_LIBRARY(simclist SHARED simclist.c)
SET(CMAKE_C_FLAGS "-O2 -Wall -std=c99")
