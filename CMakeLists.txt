INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR})
## Must include binary dir for config.h

## If you will be using the ROSSIO library add this
#INCLUDE_DIRECTORIES(${ROSSIO_SOURCE_DIR})

# Files you have included in your model
SET(model_srcs
	assist.c
	assist.h
	models/neuron_model.c
	models/neuron_model.h
	models/synapse.h
	spike_generator.c
	spike_generator.h
	mappings.c
	model_main.h
	model_main.c
	
)


# Main function (in this case) is found in model_main.c

ADD_EXECUTABLE(tnt_benchmark model_main.c ${model_srcs} mappings.h)


TARGET_LINK_LIBRARIES(tnt_benchmark ROSS m)
## Alternatively, use this command with ROSSIO
#TARGET_LINK_LIBRARIES(sr-model ROSS ROSSIO m)
