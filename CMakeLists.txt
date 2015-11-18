INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR})
## Must include binary dir for config.h

## If you will be using the ROSSIO library add this
#INCLUDE_DIRECTORIES(${ROSSIO_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11  ")


# Files you have included in your model
SET(model_srcs
	assist.c
	assist.h
	models/neuron.c
	models/neuron.h
	models/synapse.h
	input_simulator.c
	input_simulator.h
    bioModel.h
    bioModel.c
	mapping.h
	mapping.c
	clMapping.h
	clMapping.c
	neuron_out_stats.h
	neuron_out_stats.c
	model_main.h
	model_main.c

)


# Main function (in this case) is found in model_main.c

ADD_EXECUTABLE(tnt_benchmark model_main.c ${model_srcs})


TARGET_LINK_LIBRARIES(tnt_benchmark ROSS m)
## Alternatively, use this command with ROSSIO
#TARGET_LINK_LIBRARIES(sr-model ROSS ROSSIO m)
