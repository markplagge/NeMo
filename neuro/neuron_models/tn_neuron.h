//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_TN_NEURON_H
#define NEMO_TN_NEURON_H
#include "../../globals.h"
typedef struct TN_MODEL{

	//64
    tw_stime lastActiveTime; /**< last time the neuron fired - used for calculating leak and reverse functions. Should be a whole number (or very close) since big-ticks happen on whole numbers. */
    tw_stime lastLeakTime;/**< Timestamp for leak functions. Should be a mostly whole number, since this happens once per big tick. */

    //stat_type fireCount; //!< count of this neuron's output
    stat_type rcvdMsgCount; //!<  The number of synaptic messages received.
    stat_type SOPSCount; //!<  A count for SOPS calculation
    id_type myCoreID; //!< Neuron's coreID

    id_type dendriteCore; //!< Local core of the remote dendrite
    tw_lpid dendriteGlobalDest; //!< GID of the axon this neuron talks to. @todo: The dendriteCore and dendriteLocal values might not be needed anymroe.

    //32
    volt_type membranePotential; //!< current "voltage" of neuron, \f$V_j(t)\f$. Since this is PDES, \a t is implicit
    thresh_type posThreshold; //!< neuron's threshold value 𝛼
    thresh_type negThreshold; //!< neuron's negative threshold, 𝛽
    unsigned int myLocalID; //!< Neuron's local ID (from 0 - j-1);

    //16
    uint16_t dendriteLocal; //!< Local ID of the remote dendrite -- not LPID, but a local axon value (0-i)
    uint16_t drawnRandomNumber; //!<When activated, neurons draw a new random number. Reset after every big-tick as needed.
    uint16_t thresholdPRNMask;/**!< The neuron's random threshold mask - used for randomized thresholds ( \f$M_j\f$ ).
                               *	In the TN hardware this is defined as a ones maks of configurable width, starting at the
                               * least significant bit. The mask scales the range of the random drawn number (PRN) of the model,
                               * here defined as @link drawnRandomNumber @endlink. used as a scale for the random values. */
        //@TODO : Replace short with uint16_t for consistancy.
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




    char axonTypes[256];
    char synapticWeight[4];
    bool synapticConnectivity[256]; //!< is there a connection between axon i and neuron j?
    /** stochastic weight mode selection. $b_j^{G_i}$ */
    bool weightSelection[4];






    //TODO - print address offsets fof structs for performance


    //uint32_t PRNSeedValue; //!< pseudo random number generator seed. @TODO: Add PRNSeedValues to the neurons to improve TN compatibility.




}true_north_model;

#endif //NEMO_TN_NEURON_H
