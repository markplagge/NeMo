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
 
#include "ross.h"
#include <stdbool.h>
#define ARRSIZE 65537

/** typedef NeuronFireMode
 * Just in case there are multiple fire modes, this enum exists to differentiate
 *them.
 *
 * */
	typedef enum NeuronFireMode {
  NFM = 0  // normal fire mode
	} neuronFireMode;

/** \struct leakFunDel
 *	This is a dec. of a function that allows for neurons to have different
 *leak functions. At this point,
 *	the only function is a dummy one.
 *	The functions alter the neuron's current voltage.
 */
typedef void (*leakFunDel)(void *neuronState, tw_stime now);

/**
 *  @brief  noLeak - A non leaking neuron function.
 *
 *  @param neuron The current neuron state.
 *  @param now The current simulation time.
 */
void noLeak(void *neuron, tw_stime now);
/**
 *  @brief  A linear leak function - uses monotonic up and down leaks.
 *
 *  @param neuron The current neuron state.
 *  @param now The current simulation time.
 */
void linearLeak(void *neuron, tw_stime now);

void monotonicUpLeak(void *neuron, tw_stime now);
void monotonicDownLeak(void *neuron, tw_stime now);

void divergentLeak(void *neuron, tw_stime now);
void convergentLeak(void *neuron, tw_stime now);

/** \struct reverseLeakDel
 This fun. pointer manages reverse leak functions
 */
typedef void (*reverseLeakDel)(void *neuron, tw_stime now);

/**
 *  @brief  Reverse leak function for use when neurons have no defined leak function.
 *
 *  @param neuron current neuron state
 *  @param now         tw_stime representing current time of simulation.
 */
void revNoLeak(void *neuron, tw_stime now);

/**
 *  @brief  Reverse leak function neurons that have a linear leak function assigned.
 *
 *  @param neuron current neuron state
 *  @param now         tw_stime representing current time of simulation.
 */
void revLinearLeak(void *neuron, tw_stime now);

/** ResetRate
 * 	This is a support union for neuron reset rates. */

typedef union ResetRate {
	int linearRate;
	float nonLinearRate;
	_voltT voltRate;
} resetRate;

/** ResetFunDel - This is a function that handles different reset rate
 * calculations. It takes the state of the neuron, and applies
 * 	various reset functions to the neuron's voltage. Some reset functions
 * described by true north include a zeroing
 * 	function (standard integrate and fire), a linear drop function, and a
 * non-reduction function.
 * 	Also functions for leaks below. */

typedef void (*resetFunDel)(void *neuronState);
/**
 *  @brief  Resets neuron voltage to \f$R\f$ after firing. 
 *  a normal reset mode function.
 *
 *  @param neuronState current neuron state
 *
 *
 */
void resetNormal(void *neuronState);

/**
 *  @brief  Resets neuron voltage based on linear function.
 *
 *  @param neuronState neuronState
 */
void resetLinear(void *neuronState);
/**
 *  @brief  No reset function - does not reset membrane potential after firing.
 *
 *  @param neuronState current neuron state.
 */
void resetNone(void *neuronState);



/** \typedef reverseResetDel
 This is a function that reverses the reset command.
 Run first, since the reset function is run last.
 */

typedef void (*reverseResetDel)(void *neuronState);

void reverseResetLinear(void *neuronState);

void reverseResetNormal(void *neuronState);

void reverseResetNone(void *neuronState);



/**
* This struct maintains the state of an individual neuron.The neuron struct
*contains the parameters needed to maintain
* state in the neuron, along with references to output commands (dendrites).
*
* Each parameter contained within \cite{Cassidy2013}\cite{Preissl2012}\cite{Amir2013}'s models of Neuromporphic design that operate with the neuron are contained within this struct.
* Consider this struct a proto-object, just sans functions.
*
*/
typedef struct NeuronModel {
	/**@{*/

		//IDs and Lookup info
	_idT myCoreID; //!< Neuron's coreID
	_idT myLocalID; //!< Neuron's local ID (from 0 - j-1);
	/**@}*/
	/**@{*/
		//Proper state information
	_voltT membranePot; //!< current "voltage" of neuron, \f$V_j(t)\f$. Since this is PDES, \a t is implicit
	_voltT savedMembranePot; //!< previous state membrane potential
	_threshT threshold; //!< neuron's threshold value 𝛼
	_threshT negativeThreshold; //!< neuron's negative threshold, 𝛽
	uint16_t thresholdPRNMask; /**!< The neuron's random threshold mask - used for randomized thresholds ( \f$M_j\f$ ).
				     *	In the TN hardware this is defined as a ones maks of configurable width, starting at the
				     * least significant bit. The mask scales the range of the random drawn number (PRN) of the model,
				     * here defined as @link drawnRandomNumber @endlink. used as a scale for the random values. */

	uint16_t drawnRandomNumber; //!<When activated, neurons draw a new random number. Reset after every big-tick as needed.

	_voltT resetVoltage; //!< Reset voltage for reset params, \f$R\f$.
	tw_stime lastActiveTime; /**< last time the neuron fired - used for calculating leak and reverse functions. Should be a whole number (or very close) since big-ticks happen on whole numbers. */
	tw_stime lastLeakTime;/**< Timestamp for leak functions. Should be a mostly whole number, since this happens once per big tick. */

	tw_stime savedLastActiveTime;//!< For state rollback - this the last time the neuron integrated and fired, before the new big-tick
	tw_stime savedLastLeakTime; //!< For state rollback - saved last time neuron used leak function
	uint_fast16_t receivedSynapseMsgs; /**< Used for big-tick synchronization. 
		If this neuron has received a synapse message during this big-tick cycle, this will be set to > 0. Every synapse received until the big tick occurs will increment this value. Reverse events decrement this value.
		If the value is == 0 when a synapse message is received, the neuron will send a fire schedule message to itself at the next big-tick time. */
	/**@}*/

	/**@{*/

	/* neuron firing parameters */
	///@todo may not be needed for model simulation.
	neuronFireMode fireMode; ///neuron's firing mode


	resetFunDel doReset; //!< neuron reset function - has three possible values: normal, linear, non-reset: 𝛾
		//** as a test, this is the 𝛾 value - trying out mathematical reset style */
	short int resetMode;
	bool negThresReset; //!< From the paper's ,\f$𝜅_j\f$, negative threshold setting to reset or saturate
	reverseResetDel reverseReset; //!< Neuron reverse reset function.
	/**@}*/
		//Weight parameters
		//_weightT synapticWeightProb[ARRSIZE];
	/**< In this simulation, each synappse can have a unique weight. In the paper, there is a limit of four different "types" of synapse behavior per neruon. For an accurate sim, there can only be four different values in this array.
		Since this is an array, this simulator has the potential to have more power than the actual TrueNorth hardware architecture.
		The paper defines this as \f$S_j^{G_i}\f$ */

		//bool synapticWeightProbSelect[ARRSIZE];
	/**< An array determining if each synapse is handled stochastically or deterministically.
	 * Since the actual hardware has 4 synapse types, this setup has more power than the actual
	 *  TrueNorth architecture.
	 * To ensure model <-> hardware accuracy, at most four different modes should be used per neuron,
	 *  so that synapses are handled as one of four possible types.
		The paper defines this as \f$b_j^{G_i}\f$
*/

	_weightT axonWeightProb[4];
	bool axonProbSelect[4];
	unsigned char weightSelect[256];

	    //Output locations:
	_idT dendriteCore; //!< Local core of the remote dendrite
	uint16_t dendriteLocal; //!< Local ID of the remote dendrite -- not LPID, but a local axon value (0-i)
	tw_lpid dendriteGlobalDest; //!< GID of the axon this neuron talks to. @todo: The dendriteCore and dendriteLocal values might not be needed anymroe.

		//Leak functionality
	leakFunDel doLeak; //!< Function pointer to the neuron's current leak function.
	reverseLeakDel doLeakReverse; //!< Function pointer to the leak reverse function

	_weightT leakRateProb; //!< \f$𝜆\f$ Leak tuning parameter - the leak rate applied to the current leak function.
	bool leakWeightProbSelect; //!< If true, this is a stochastic leak function and the \a leakRateProb value is a probability, otherwise it is a leak rate.

	bool leakReversalFlag; //!< from the paper this changes the function of the leak from always directly being integrated (false), or having the leak directly integrated when membrane potential is above zero, and the sign is reversed when the membrane potential is below zero, with zero not allowing a leak.

		//Stats
	_statT fireCount; //!< count of this neuron's output
	_statT rcvdMsgCount; //!<  The number of synaptic messages received.
	_statT SOPSCount; //!<  A count for SOPS calculation

/**@}*/
	bool firedLast; //!< Did the neuron fire during the last message?

}neuronState;

/* ***Neuron functions */
/**
 * @brief neuron reversal function.
 * Used to roll back any calls made by the neuron. Decrements receivedSynapseMsgs Reset funs have already
 * been run at this point @see reverseLeakDel() and @see reverseResetDel()
 * @param s the neuron state
 * @param CV transported bitfield
 * @param m the rollback message
 * @param lp the lp
 */
void neuornReverseState(neuronState *s, tw_bf *CV,Msg_Data *m,tw_lp *lp);

/**
 *  @brief  handles incomming synapse messages. In this model, the neurons send messages to axons during "big tick" intervals.
 This is done through an event sent upon receipt of the first synapse message of the current big-tick.
 *
 *  @param st   current neuron state
 *  @param time time event was received
 *  @param m    event message data
 *  @param lp   lp.
 */
void neuronReceiveMessage(neuronState *st, tw_stime time, Msg_Data *m,
						  tw_lp *lp);
void neuronReceiveMessageBasic(neuronState *st, tw_stime time, Msg_Data *m,
			       tw_lp *lp);
/** neuronFire manages a firing event. Firing events occur when a synchro message is received, so these calculations are done on big-ticks only. */
void nSpike(neuronState *st, tw_stime time, tw_lp *lp);


/**
 *  @brief  function that adds a synapse's value to the current neuron's membrane potential.
 *
 *  @param synapseID localID of the synapse sending the message.
 */
void integrateSynapse(_idT synapseID,neuronState *st, tw_lp *lp);

void integrateSynapseFast(_idT axonID, neuronState *st, tw_lp *lp);

/**
 *  @brief  Function that sends a heartbeat message to this neuron.
 *
 *  @param lp   <#lp description#>
 *  @param time <#time description#>
 */
void sendHeartbeat(neuronState *st, tw_lp *lp, tw_stime time);

/**
 *  @brief  Checks to see if a neuron should fire. @todo check to see if this is needed, since it looks like just a simple if statement is in order.
 *
 *  @param st neuron state
 *
 *  @return true if the neuron is ready to fire.
 */
bool neuronShouldFire(neuronState *st, tw_lp *lp);

/**
 *  @brief  Function that runs after integration & firing, for reset function and threshold bounce calls.
 *
 *  @param st      state
 *  @param time    event time
 *  @param lp      lp
 *  @param didFire did the neuron fire during this big tick?
 */
void neuronPostIntegrate(neuronState *st, tw_stime time, tw_lp *lp, bool willFire);
/**
 *  @brief  Neuron stochastic integration function - for use with stochastic leaks and synapse messages.
 *
 *  @param weight weight of selected leak or synapse
 *  @param st     the neuron state
 */
void stochasticIntegrate(_weightT weight, neuronState *st);



#endif /* defined(__ROSS_TOP__neuron__) */
