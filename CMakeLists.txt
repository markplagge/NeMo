INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR})
## Must include binary dir for config.h

## If you will be using the ROSSIO library add this
#INCLUDE_DIRECTORIES(${ROSSIO_SOURCE_DIR})

# Files you have included in your model
SET(model_srcs
	model_driver.c  
	
	assist.c
	assist.h
	libs\simclist.h
	libs\simclist.c
	models\neuron_model.c
	models\neuron_model.h
	models\synapse.h
	models\model.h
)

# Set the executable name here: sr-model
# You will be executing the ./sr-model binary
# Main function (in this case) is found in model_main.c

ADD_EXECUTABLE(tnt_benchmark model_main.c ${model_srcs})


TARGET_LINK_LIBRARIES(sr-model ROSS m)
## Alternatively, use this command with ROSSIO
#TARGET_LINK_LIBRARIES(sr-model ROSS ROSSIO m)
