//
// Created by plaggm on 3/18/15.
//
/** Simplified neuron model for benchmarking */
#ifndef _TNT_MAIN_NEURON_MODEL_H_
#define _TNT_MAIN_NEURON_MODEL_H_

#ifdef    __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "assist.h"
#include "ross.h" //trying to minimize calls to ross from here, but certian defs are really needed.


/** typedef NeuronFireMode
* Just in case there are multiple fire modes, this enum exists to differentiate them.
*
* */
typedef enum NeuronFireMode{
	NFM = 0 // normal fire mode (if voltage > threshold, fire);
} neuronFireMode;


/** \struct leakFunDel
*	This is a dec. of a function that allows for neurons to have different leak functions. At this point,
*	the only function is a dummy one.
*	The functions alter the neuron's current voltage.
*/
typedef void (*leakFunDel)(void *neuronState, tw_stime end);
void noLeak(void *neuronState,  tw_stime end);

	/** \struct reverseLeakDel
	 This fun. pointer manages reverse leak functions
	 */

	typedef void(*reverseLeakDel)(void *neuronState, tw_stime now);
	void revNoLeak(void *neuronState, tw_stime now);
/** \union ResetRate
* 	This is a support union for neuron reset rates. */

union ResetRate{
   int linearRate;
   float nonLinearRate;
}resetRate;

/** ResetFunDel - This is a function that handles different reset rate calculations. It takes the state of the neuron, and applies
* 	various reset functions to the neuron's voltage. Some reset functions described by true north include a zeroing
* 	function (standard integrate and fire), a linear drop function, and a non-reduction function.
* 	Also functions for leaks below. */

typedef void (*resetFunDel)(void *neuronState);
//zero function (includes implementation due to size;
void resetZero(void *neuronState);
//linear function
void resetLinear(void *neuronState);

	/** \typedef reverseResetDel
	 This is a function that reverses the reset command. 
	 Run first, since the reset function is run last.
	 */

	typedef void (*reverseResetDel)(void *neuronState);
	void reverseLinear(void *neuronState);
	void reverseZero(void *neuronState);


 	/** \struct NeuronModel
* This struct maintains the state of an individual neuron.The neuron struct contains the parameters needed to maintain
* state in the neuron, along with references to output commands (dendrites).
*
* Each parameter contained within \cite Cassidy2013 , \cite Preissl2012 , \cite Amir2013's models of Neuromporphic design
* that operate with the neuron are contained within this struct.
* Consider this struct a proto-object, just sans functions.
*
*/
typedef struct NeuronModel {
   _idType coreID; // local coreID
   _idType neuronID; //local neuron ID (each core has some number of neurons)
   _neVoltType cVoltage; //current "voltage" contained in the neuron.
   _neVoltType prVoltage; //previous neuron voltage.
   _neVoltType threshold; //neuron's threshold voltage
   //time (for reverse functions and such) --- Maybe use tw_stime here?
   tw_stime lastActiveTime; // last time the neuron fired - used for calculating leak and reverse functions

   /** neuron fire parameters */
   _neStatType fireCount;

   neuronFireMode fireMode;
	/**Reset Function */
   resetFunDel doReset;
   _neVoltType resetVoltParam; /**< Optional parameter for reset voltage function. */


	/**Reverse reset function */
	reverseResetDel reverseReset;

   /** synapse input weights */
   //TODO: generate new type for synapse weight. (signed int)
   _neVoltType *perSynapseWeight; //each input synapse has a unique weight or probability.
   bool *perSynapseDet; //Is the input synapse a stochastic synapse?
   /** neuron leak parameters */
   tw_stime lastLeakTime;
   _neVoltType leakRate;
	_neVoltType sgnLambda;
	
   leakFunDel leak;

	/**Reverse Leak Functuon */
	reverseLeakDel reverseLeak;

   _idType dendriteLocalDest;
   _idType dendriteCore;
	tw_lpid dendriteDest;

		//Parameters from the paper:



} neuronState;

bool neuronReceiveMessage(neuronState *st, tw_stime time, Msg_Data *m, tw_lp *lp);
void neuronFire(neuronState *st, tw_stime time, Msg_Data *m);
void neuronPostFire(neuronState *st, tw_stime time, Msg_Data *m);


#ifdef        __cplusplus
}
#endif
#endif //_TNT_MAIN_NEURON_MODEL_H_
