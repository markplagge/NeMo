//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_NEURON_H
#define NEMO_NEURON_H
#include "neuron.h"
#include "globals.h"
/** ResetFunDel - This is a function that handles different reset rate
* calculations. It takes the state of the neuron, and applies
* 	various reset functions to the neuron's voltage. Some reset functions
* described by true north include a zeroing
* 	function (standard integrate and fire), a linear drop function, and a
* non-reduction function.
* 	Also functions for leaks below. */

typedef void (*resetDel)(void *neuronState);

typedef void (*integrateDel)(void *neuronState);

typedef void (*leakDel)(void *neuronState);

typedef void (*reverseResetDel)(void *neuronState, void *messageData);

typedef void (*reverseIntegrateDel)(void *neuronState, void *messageData);

typedef void (*reverseLeakDel)(void *neuronState, void *messageData);



typedef struct NeuronModel {
	/**{ */
	resetDel reset;//!< neuron reset function 
	integrateDel integrate; //!< neuron integration function
	leakDel leak; //!< neuron leak function
	reverseResetDel reverseReset;
	reverseIntegrateDel reverseIntegrate;
	reverseLeakDel reverseLeak;
	/**}*/

	
	void* data;
	


}neuronState;


#endif //NEMO_NEURON_H
