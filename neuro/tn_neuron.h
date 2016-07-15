//
// Created by Mark Plagge on 5/25/16.
//

#ifndef __NEMO_TN_NEURON_H__
#define __NEMO_TN_NEURON_H__



#include "../globals.h"
#include "../mapping.h"
#include "../IO/IOStack.h"
#include <math.h>
#define Vj ns->membranePotential

#ifdef SAVE_MSGS




#endif

#ifdef SAVE_NEURON_STATS



#endif

/**
 * TrueNorth LP Neuron Model struct
 */
typedef struct TN_MODEL{

	//64
    tw_stime lastActiveTime; /**< last time the neuron fired - used for calculating leak and reverse functions. Should be a whole number (or very close) since big-ticks happen on whole numbers. */
    tw_stime lastLeakTime;/**< Timestamp for leak functions. Should be a mostly whole number, since this happens once per big tick. */

    tw_lpid outputGID; //!< The output GID (axon global ID) of this neuron.


    //stat_type fireCount; //!< count of this neuron's output
    stat_type rcvdMsgCount; //!<  The number of synaptic messages received.
    stat_type SOPSCount; //!<  A count for SOPS calculation
                         
                          
    

    //32
    volt_type membranePotential; //!< current "voltage" of neuron, \f$V_j(t)\f$. Since this is PDES, \a t is implicit
    thresh_type posThreshold; //!< neuron's threshold value 𝛼
    thresh_type negThreshold; //!< neuron's negative threshold, 𝛽

    //16
    id_type dendriteLocal; //!< Local ID of the remote dendrite -- not LPID, but a local axon value (0-i)
    random_type drawnRandomNumber; //!<When activated, neurons draw a new random number. Reset after every big-tick as needed.
    random_type thresholdPRNMask;/**!< The neuron's random threshold mask - used for randomized thresholds ( \f$M_j\f$ ).
                               *	In the TN hardware this is defined as a ones maks of configurable width, starting at the
                               * least significant bit. The mask scales the range of the random drawn number (PRN) of the model,
                               * here defined as @link drawnRandomNumber @endlink. used as a scale for the random values. */

    id_type myCoreID; //!< Neuron's coreID
                      
    id_type myLocalID;//!< my local ID - core wise. In a 512 size core, neuron 0 would have a local id of 262,657. 


    //small
    short largestRandomValue;
    short lambda; //!< leak weight - \f$𝜆\f$ Leak tuning parameter - the leak rate applied to the current leak function.
    short int resetMode; //!< Reset mode selection. Valid options are 0,1,2 . Gamma or resetMode 𝛾
    volt_type resetVoltage; //!< Reset voltage for reset params, \f$R\f$.
    short sigmaVR; //!< reset voltage - reset voltage sign
    short encodedResetVoltage; //!< encoded reset voltage - VR.
    short omega; //!<temporary leak direction variable

    char* neuronTypeDesc; //!< a debug tool, contains a text desc of the neuron.
    char sigma_l; //!< leak sign bit - eqiv. to σ
    unsigned char delayVal; //!<@todo: Need to fully implement this - this value is between 1 and 15, a "delay" of n timesteps of a neuron. -- outgoing delay //from BOOTCAMP!

    //@TODO - convert this to a bitfield for bools. Check ROSS BF implementation
    
    bool firedLast;
    bool heartbeatOut;
    bool isSelfFiring;
    bool epsilon; //!<epsilon function - leak reversal flag. from the paper this changes the function of the leak from always directly being integrated (false), or having the leak directly integrated when membrane potential is above zero, and the sign is reversed when the membrane potential is below zero.
    bool c; //!< leak weight selection. If true, this is a stochastic leak function and the \a leakRateProb value is a probability, otherwise it is a leak rate.
    bool kappa; //!<Kappa or negative reset mode. From the paper's ,\f$𝜅_j\f$, negative threshold setting to reset or saturate
    bool canGenerateSpontaniousSpikes;



	
    char axonTypes[512];
    char synapticWeight[4];
    bool synapticConnectivity[512]; //!< is there a connection between axon i and neuron j?
    /** stochastic weight mode selection. $b_j^{G_i}$ */
    bool weightSelection[4];






    //TODO - print address offsets fof structs for performance


    //uint32_t PRNSeedValue; //!< pseudo random number generator seed. @TODO: Add PRNSeedValues to the neurons to improve TN compatibility.




}tn_neuron_state;




/**
 * @brief      True North Forward Event handler
 *
 * @param      s  The tn neuron state
 * @param      CV               flags for message flow
 * @param      messageData      The message data
 * @param      lp               The pointer to a LP
 */
void TN_forward_event (tn_neuron_state *s, tw_bf *CV, messageData *m,
    tw_lp *lp);

/**
 * @brief      True North Reverse Event Handler
 *
 * @param      s  The tn neuron state
 * @param      CV               flags for message flow
 * @param      messageData      The message data
 * @param      lp               The pointer to a
 */
void TN_reverse_event (tn_neuron_state *s, tw_bf *CV, messageData *m,
    tw_lp *lp);


/**
 * @brief      Initialize a TrueNorth neuron
 *
 * @param      s     The TN State
 * @param      lp    The pointer to the LP
 */
void TN_init(tn_neuron_state *s, tw_lp *lp);

/**
 * @brief      The TN neuron final function
 *
 * @param      s     TN State
 * @param      lp    The pointer to an LP
 */
void TN_final(tn_neuron_state *s, tw_lp *lp);


/**
 * @brief	This takes a void pointer and returns this neuron's struct. 
 * This is used for managing super synapse direct communication functionality.
 */

inline tn_neuron_state * TN_convert(void * lpstate);


#endif //NEMO_TN_NEURON_H
