//
//  neuron.h
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#ifndef __ROSS_TOP__neuron__
#define __ROSS_TOP__neuron__

#include <stdio.h>
#include "../assist.h"
#include "../mapping.h"
#include "ross.h"
#include <stdbool.h>

/** typedef NeuronFireMode
 * Just in case there are multiple fire modes, this enum exists to differentiate
 *them.
 *
 * */
	typedef enum NeuronFireMode {
  NFM = 0  // normal fire mode (if voltage > threshold, fire);
	} neuronFireMode;

/** \struct leakFunDel
 *	This is a dec. of a function that allows for neurons to have different
 *leak functions. At this point,
 *	the only function is a dummy one.
 *	The functions alter the neuron's current voltage.
 */
typedef void (*leakFunDel)(void *neuronState, tw_stime end);
void noLeak(void *neuronState, tw_stime end);

/** \struct reverseLeakDel
 This fun. pointer manages reverse leak functions
 */

typedef void (*reverseLeakDel)(void *neuronState, tw_stime now);
void revNoLeak(void *neuronState, tw_stime now);
/** \union ResetRate
 * 	This is a support union for neuron reset rates. */

union ResetRate {
	int linearRate;
	float nonLinearRate;
} resetRate;

/** ResetFunDel - This is a function that handles different reset rate
 * calculations. It takes the state of the neuron, and applies
 * 	various reset functions to the neuron's voltage. Some reset functions
 * described by true north include a zeroing
 * 	function (standard integrate and fire), a linear drop function, and a
 * non-reduction function.
 * 	Also functions for leaks below. */

typedef void (*resetFunDel)(void *neuronState);
	// zero function (includes implementation due to size;
void resetZero(void *neuronState);
	// linear function
void resetLinear(void *neuronState);

/** \typedef reverseResetDel
 This is a function that reverses the reset command.
 Run first, since the reset function is run last.
 */

typedef void (*reverseResetDel)(void *neuronState);
void reverseLinear(void *neuronState);
void reverseZero(void *neuronState);


/** \struct NeuronModel
* This struct maintains the state of an individual neuron.The neuron struct
*contains the parameters needed to maintain
* state in the neuron, along with references to output commands (dendrites).
*
* Each parameter contained within \cite Cassidy2013 , \cite Preissl2012 , \cite
*Amir2013's models of Neuromporphic design
* that operate with the neuron are contained within this struct.
* Consider this struct a proto-object, just sans functions.
*
*/
typedef struct NeuronModel {
		//IDs and Lookup info
	_idT myCoreID; ///Neuron's coreID
	_idT myLocalID; ///Neuron's local ID (from 0 - j-1);

		//Proper state information
	_voltT membranePot; ///current "voltage" of neuron
	_voltT prevMembranePot; ///previous state membrane potential
	_voltT threshold; ///neuron's threshold value
	tw_stime lastActiveTime; ///last time the neuron fired - used for calculating leak and reverse functions.
	uint_fast16_t receivedSynapseMsgs; /**< Used for big-tick synchronization. 
		If this neuron has received a synapse message during this big-tick cycle, this will be set to > 0. Every synapse received until the big tick occurs will increment this value. Reverse events decrement this value.
		If the value is == 0 when a synapse message is received, the neuron will send a fire schedule message to itself at the next big-tick time. */


	/** neuron firing parameters */
	neuronFireMode fireMode; ///neuron's firing mode

		/** neuron reset params */
	resetFunDel doReset; /// neuron reset function
	_voltT resetVoltParam; ///Optional parameter for reset voltage functions

	reverseResetDel reverseReset; ///Neuron reverse reset function.

		//Weight parameters
	_voltT *perSynapseWeight; /**< In this simulation, each synappse can have a unique weight. In the paper, there is a limit of four different "types" of synapse behavior per neruon. For an accurate sim, there can only be four different values in this array.

		Since this is an array, this simulator has the potential to have more power than the actual TrueNorth hardware architecture. */
	bool *perSynapseDet; /**< An array determining if each synapse is handled stochastically or deterministically. Since the actual hardware has 4 synapse types, this setup has more power than the actual TrueNorth architecture.

		To ensure model <-> hardware accuracy, at most four different modes should be used per neuron, so that synapses are handled as one of four possible types. */

		//Output locations:
	_idT dendriteCore; ///Local core of the remote dendrite
	_idT dendriteLocal; ///Local ID of the remote dendrite -- not LPID, but a local axon value (0-i)
	tw_lpid dendriteGlobalDest; ///GID of the axon this neuron talks to. @TODO: The dendriteCore and dendriteLocal values might not be needed anymroe.

		//Leak functionality
	leakFunDel doLeak; ///Function pointer to the neuron's current leak function.
	reverseLeakDel doLeakReverse; //Function pointer to the leak reverse function

	_voltT leakRate; //Leak tuning parameter - the leak rate applied to the current leak function.
	_voltT sgnLambda; //sgnLambda tuning parameter from the paper - used for specific leak functions.

		//Stats
	_statT fireCount; ///count of this neuron's output
	_statT rcvdMsgCount; /// The number of synaptic messages received.
	_statT SOPSCount; /// A count for SOPS calculation


}neuronState;
/* ***Neuron functions */
/**
 * @brief neuronReverseFinal final neuron reversal function.
 * Used to roll back any calls made by the neuron. Decrements receivedSynapseMsgs Reset funs have already
 * been run at this point @see reverseLeakDel() and @see reverseResetDel()
 * @param s the neuron state
 * @param CV transported bitfield
 * @param m the rollback message
 * @param lp the lp
 */
void neuronReverseFinal(neuronState *s, tw_bf *CV,Msg_Data *m,tw_lp *lp);
	/** neuronReceiveMessage handles incomming synapse messages. In this model, the neurons send messages to axons during "big tick" intervals. 
	 This is done through an event sent upon receipt of the first synapse message of the current big-tick. */
void neuronReceiveMessage(neuronState *st, tw_stime time, Msg_Data *m,
						  tw_lp *lp);
/** neuronFire manages a firing event. Firing events occur when a synchro message is received, so these calculations are done on big-ticks only. */
void neuronFire(neuronState *st, tw_stime time, Msg_Data *m);
/** neuronPostFire manages post-firing events, including reset functions */
void neuronPostFire(neuronState *st, tw_stime time, Msg_Data *m);
/**generateWaitEvent creates a new wait event to this neuron for big-tick synchronization */
void generateWaitEvent(neuronState *st, tw_stime time, tw_lp *lp);

#endif /* defined(__ROSS_TOP__neuron__) */
