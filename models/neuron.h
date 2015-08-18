//
//  neuron.h
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#ifndef __ROSS_TOP__neuron__
#define __ROSS_TOP__neuron__

#include "../assist.h"
 
#include "ross.h"
#include <stdbool.h>



//Externs:
extern int NEURONS_IN_CORE;
extern unsigned int CORES_IN_SIM;
extern int AXONS_IN_CORE;
extern int SYNAPSES_IN_CORE;


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



typedef struct NeuronModel {
    /*@{ #name RossInfo */
    
    bool firedLast;
    bool heartbeatOut;
    bool isSelfFiring;
    long long receivedSynapseMsgs;
    
    /*@} */
     /*@{ @name location Information */
    id_type myCoreID; //!< Neuron's coreID
    id_type myLocalID; //!< Neuron's local ID (from 0 - j-1);
    
    
    /*@} */
    /*@{ @name Signal Sending Parameters */
    unsigned char delayVal; //!<@todo: Need to fully implement this - this value is between 1 and 15, a "delay" of n timesteps of a neuron. -- outgoing delay //from BOOTCAMP!
    id_type dendriteCore; //!< Local core of the remote dendrite
    uint16_t dendriteLocal; //!< Local ID of the remote dendrite -- not LPID, but a local axon value (0-i)
    tw_lpid dendriteGlobalDest; //!< GID of the axon this neuron talks to. @todo: The dendriteCore and dendriteLocal values might not be needed anymroe.
    
    /*@}*/
    
    
    /*@{ @name Weight & Threshold Parameters */
    volt_type membranePotential; //!< current "voltage" of neuron, \f$V_j(t)\f$. Since this is PDES, \a t is implicit
    uint32_t synapticConnectivity[256]; //!< is there a connection between axon i and neuron j?
    
    char axonTypes[256];
    
    short synapticWeight[4];
    short weightSelection[4];
    thresh_type posThreshold; //!< neuron's threshold value 𝛼
    thresh_type negThreshold; //!< neuron's negative threshold, 𝛽
    uint16_t thresholdPRNMask;/**!< The neuron's random threshold mask - used for randomized thresholds ( \f$M_j\f$ ).
                               *	In the TN hardware this is defined as a ones maks of configurable width, starting at the
                               * least significant bit. The mask scales the range of the random drawn number (PRN) of the model,
                               * here defined as @link drawnRandomNumber @endlink. used as a scale for the random values. */
    uint16_t drawnRandomNumber; //!<When activated, neurons draw a new random number. Reset after every big-tick as needed.
    short thresholdMaskBits; //!< TM, or the number of bits for the random number. Use this to generate the thresholdPRN mask;
    
    short largestRandomValue;
    
    /*@} */
    
    /*@{ @name Timing Parameters */
    tw_stime lastActiveTime; /**< last time the neuron fired - used for calculating leak and reverse functions. Should be a whole number (or very close) since big-ticks happen on whole numbers. */
    tw_stime lastLeakTime;/**< Timestamp for leak functions. Should be a mostly whole number, since this happens once per big tick. */
    /*@}*/
    
    /**@{ @name Integrate, Leak, and Fire Parameters */
    
    //leak params -- could actually turn these into one bitfield rather than having all of these wasted bits:
    
    bool epsilon; //!<epsilon function - leak reversal flag. from the paper this changes the function of the leak from always directly being integrated (false), or having the leak directly integrated when membrane potential is above zero, and the sign is reversed when the membrane potential is below zero.
    
    short sigma_l; //!< sigma param - leak sign
    short lambda; //!< leak weight - \f$𝜆\f$ Leak tuning parameter - the leak rate applied to the current leak function.
    bool c; //!< leak weight selection. If true, this is a stochastic leak function and the \a leakRateProb value is a probability, otherwise it is a leak rate.
    
    resetFunDel doReset; //!< neuron reset function - has three possible values: normal, linear, non-reset: 𝛾
    
    short int resetMode; //!< Reset mode selection. Valid options are 0,1,2 . Gamma or resetMode 𝛾
    volt_type resetVoltage; //!< Reset voltage for reset params, \f$R\f$.
    short sigmaVR; //!< reset voltage - reset voltage sign
    short encodedResetVoltage; //!< encoded reset voltage - VR.
    bool kappa; //!<Kappa or negative reset mode. From the paper's ,\f$𝜅_j\f$, negative threshold setting to reset or saturate
    short omega; //!<temporary leak direction variable
    
    uint32_t PRNSeedValue; //!< pseudo random number generator seed.
    
    /**@}*/
    /**@{*/
    stat_type fireCount; //!< count of this neuron's output
    stat_type rcvdMsgCount; //!<  The number of synaptic messages received.
    stat_type SOPSCount; //!<  A count for SOPS calculation
    
    // a helper boolean:
    bool canGenerateSpontaniousSpikes;
    
    /**@}*/
    
    
}neuronState;

/* ***Neuron functions */

void  initNeuron(id_type coreID, id_type nID,
                 bool synapticConnectivity[],
                 short G_i[], int sigma[4], int S[4], bool b[4], bool epsilon,
                 short sigma_l, short lambda, bool c, uint32_t alpha,
                 uint32_t beta, short TM, short VR, short sigmaVR, short gamma, bool kappa, neuronState *n, int signalDelay, uint64_t destGlobalID);
/**
 *  @brief  handles incomming synapse messages. In this model, the neurons send messages to axons during "big tick" intervals.
 This is done through an event sent upon receipt of the first synapse message of the current big-tick.
 *
 *  @param st   current neuron state
 *  @param time time event was received
 *  @param m    event message data
 *  @param lp   lp.
 */
void neuronReceiveMessage(neuronState *st, Msg_Data *M, tw_lp *lp);
/**
 *  @brief  function that adds a synapse's value to the current neuron's membrane potential.
 *
 *  @param synapseID localID of the synapse sending the message.
 */
void integrate(id_type synapseID,neuronState *st, void *lp);

/**
 *  @brief  Function that sends a heartbeat message to this neuron.
 *
 *  @param lp   <#lp description#>
 *  @param time
 */
void sendHeartbeat(neuronState *st,  tw_stime time ,void *lp) ;

/**
 *  @brief  Checks to see if a neuron should fire. @todo check to see if this is needed, since it looks like just a simple if statement is in order.
 *
 *  @param st neuron state
 *
 *  @return true if the neuron is ready to fire.
 */
bool neuronShouldFire(neuronState *st, void *lp);

/**
 *  @brief  Function that runs after integration & firing, for reset function and threshold bounce calls.
 *
 *  @param st      state
 *  @param time    event time
 *  @param lp      lp
 *  @param didFire did the neuron fire during this big tick?
 */
void neuronPostIntegrate(neuronState *st, tw_stime time, tw_lp *lp, bool didFire);
/**
 *  @brief  Neuron stochastic integration function - for use with stochastic leaks and synapse messages.
 *
 *  @param weight weight of selected leak or synapse
 *  @param st     the neuron state
 */
void stochasticIntegrate(weight_type weight, neuronState *st);

void setNeuronDest(int signalDelay, uint64_t globalID, neuronState *n);

void numericLeakCalc(neuronState *st, tw_stime now);

void fire(neuronState *st, void *lp);

void setNeuronDest(int signalDelay, uint64_t gid, neuronState *n);

void neuronReverseState(neuronState *s, tw_bf *CV, Msg_Data *m, tw_lp *lp);

#endif /* defined(__ROSS_TOP__neuron__) */
