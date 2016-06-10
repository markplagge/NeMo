//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_NEURON_H
#define NEMO_NEURON_H

//#include "../globals.h"
#include "../bio_validation.h"
#include "./neuron_models/tn_neuron.h"


void initHandlers(int neuronType, int synapseType, int axonType);

void neuron_init(neuronState *s, tw_lp *lp);
void neuron_event(neuronState *s, tw_bf *CV, messageData *M, tw_lp *lp);
void neuron_reverse(neuronState *, tw_bf *, messageData *, tw_lp *);
void neuron_final(neuronState *s, tw_lp *lp);

/**
 * \defgroup IBMFunctions IBM Function handlers
 * @{
 */
/**
 * @brief      creates an IBM Neuron
 *
 * @param      s     Neuron State
 * @param      lp    The pointer to a
 */
void IBM_neuron_init(neuronState *s, tw_lp *lp);

void IBM_neuron_event(neuronState *s, tw_bf *CV, messageData *M, tw_lp *lp);
void IBM_neuron_reverse(neuronState *, tw_bf *, messageData *, tw_lp *);
void IBM_neuron_final(neuronState *s, tw_lp *lp);


void IBM_synapse_init(synapseState *s, tw_lp *lp);
void IBM_synapse_event(synapseState *s, tw_bf *, messageData *M, tw_lp *lp);
void IBM_synapse_reverse(synapseState *, tw_bf *, messageData *M, tw_lp *);
void IBM_synapse_final(synapseState *s, tw_lp *lp);

void IBM_axon_init(axonState *s, tw_lp *lp);
void IBM_axon_event(axonState *s, tw_bf *, messageData *M, tw_lp *lp);
void IBM_axon_reverse(axonState *, tw_bf *, messageData *M, tw_lp *);
void IBM_axon_final(axonState *s, tw_lp *lp);


void IBM_create_simple_neuron(neuronState *s, tw_lp *lp);
/** Creates a neuron using standard spiking parameters. Reset voltage is  calculated
 here as VR * sigmaVR for model compatibility */
void  IBMinitNeuron(id_type coreID, id_type nID,
                 bool synapticConnectivity[],
                 short G_i[], short sigma[4], short S[4], bool b[4], bool epsilon,
                 short sigma_l, short lambda, bool c, uint32_t alpha,
                 uint32_t beta, short TM, short VR, short sigmaVR, short gamma, bool kappa, neuronState *n, int signalDelay, uint64_t destGlobalID, int destAxonID);
/** Creates a neuron using the encoded reset value method. Use this for more
 complete compatability with TrueNorth */
void IBMinitNeuronEncodedRV(id_type coreID, id_type nID,
                         bool synapticConnectivity[NEURONS_IN_CORE],
                         short G_i[NEURONS_IN_CORE], short sigma[4],
                         short S[4], bool b[4], bool epsilon,
                         short sigma_l, short lambda, bool c, uint32_t alpha,
                         uint32_t beta, short TM, short VR, short sigmaVR, short gamma,
                         bool kappa, neuronState *n, int signalDelay, uint64_t destGlobalID,int destAxonID);
/**
 *  @brief  handles incomming synapse messages. In this model, the neurons send messages to axons during "big tick" intervals.
 This is done through an event sent upon receipt of the first synapse message of the current big-tick.
 *
 *  @param st   current neuron state
 *  @param time time event was received
 *  @param m    event message data
 *  @param lp   lp.
 */
bool IBMneuronReceiveMessage(neuronState *st, messageData *M, tw_lp *lp, tw_bf *bf);
/**
 *  @brief  function that adds a synapse's value to the current neuron's membrane potential.
 *
 *  @param synapseID localID of the synapse sending the message.
 */
void IBMintegrate(id_type synapseID,neuronState *st, void *lp);


/**
 *  @brief  Checks to see if a neuron should fire. @todo check to see if this is needed, since it looks like just a simple if statement is in order.
 *
 *  @param st neuron state
 *
 *  @return true if the neuron is ready to fire.
 */
bool IBMneuronShouldFire(neuronState *st, void *lp);

/**
 * @brief New firing system using underflow/overflow and reset.
 * @return true if neuron is ready to fire. Membrane potential is set regardless.
 */
bool IBMfireFloorCelingReset(neuronState *ns, tw_lp *lp);


/**
 *  @brief  Function that runs after integration & firing, for reset function and threshold bounce calls.
 *
 *  @param st      state
 *  @param time    event time
 *  @param lp      lp
 *  @param didFire did the neuron fire during this big tick?
 */
void IBMneuronPostIntegrate(neuronState *st, tw_stime time, tw_lp *lp, bool didFire);
/**
 *  @brief  Neuron stochastic integration function - for use with stochastic leaks and synapse messages.
 *
 *  @param weight weight of selected leak or synapse
 *  @param st     the neuron state
 */
void IBMstochasticIntegrate(weight_type weight, neuronState *st);

/**
 * @brief      configures the IBM neuron destination
 *
 * @param[in]  signalDelay  The signal delay
 * @param[in]  globalID     The global id
 * @param      n            { parameter_description }
 */
void IBMsetNeuronDest(int signalDelay, uint64_t globalID, neuronState *n);

/**
 *  @brief NumericLeakCalc - uses formula from the TrueNorth paper to calculate leak.
 *  @details Will run $n$ times, where $n$ is the number of big-ticks that have occured since
 *  the last integrate. Handles stochastic and regular integrations.
 *
 *  @TODO: self-firing neurons will not properly send messages currently - if the leak is divergent, the flag needs to be set upon neuron init.
 *  @TODO: does not take into consideration thresholds. Positive thresholds would fall under self-firing neurons, but negative thresholds should be reset.
 *  @TODO: test leaking functions
 */
void IBMnumericLeakCalc(neuronState *st, tw_stime now);

void IBMfire(neuronState *st, void *lp);

void IBMsetNeuronDest(int signalDelay, uint64_t gid, neuronState *n);

void IBMneuronReverseState(neuronState *s, tw_bf *CV, messageData *m, tw_lp *lp);

void IBMsendHeartbeat(neuronState *st, tw_stime time, void *lp);
void IBMlinearLeak(neuronState *neuron, tw_stime now);

/** @} */

typedef void (*forwardEventDel)(void *neuronState, void *messageData, void *lp, void *BF);

typedef void (*reverseEventDel)(void *neuronState, void *messageData, void *lp, void *BF);




typedef struct NeuronModel {
	/** \defgroup modelDel Neuron model function delegates.
     *@{ */
    forwardEventDel forwardEvent;
    reverseEventDel reverseEvent;
	// resetDel reset;//!< neuron reset function 
	// integrateDel integrate; //!< neuron integration function
	// leakDel leak; //!< neuron leak function
	// reverseResetDel reverseReset;
	// reverseIntegrateDel reverseIntegrate;
	// reverseLeakDel reverseLeak;
	/**@}*/
	tw_lpid outputGID;


	id_type myLocalID;
	id_type myCoreID;
	
	/**
	 * \defgroup neuronState TrueNorth neuron state data.
	 * @{
	 */


	
   

    //64
    tw_stime lastActiveTime; /**< last time the neuron fired - used for calculating leak and reverse functions. Should be a whole number (or very close) since big-ticks happen on whole numbers. */
    tw_stime lastLeakTime;/**< Timestamp for leak functions. Should be a mostly whole number, since this happens once per big tick. */

    //stat_type fireCount; //!< count of this neuron's output
    stat_type rcvdMsgCount; //!<  The number of synaptic messages received.
    stat_type SOPSCount; //!<  A count for SOPS calculation
    

    id_type dendriteCore; //!< Local core of the remote dendrite
    tw_lpid dendriteGlobalDest; //!< GID of the axon this neuron talks to. @todo: The dendriteCore and dendriteLocal values might not be needed anymroe.


    //32
    volt_type membranePotential; //!< current "voltage" of neuron, \f$V_j(t)\f$. Since this is PDES, \a t is implicit
    thresh_type posThreshold; //!< neuron's threshold value 𝛼
    thresh_type negThreshold; //!< neuron's negative threshold, 𝛽
    

    //16
    uint16_t dendriteLocal; //!< Local ID of the remote dendrite -- not LPID, but a local axon value (0-i)
    uint16_t drawnRandomNumber; //!<When activated, neurons draw a new random number. Reset after every big-tick as needed.
    uint16_t thresholdPRNMask;/**!< The neuron's random threshold mask - used for randomized thresholds ( \f$M_j\f$ ).
                               *	In the TN hardware this is defined as a ones maks of configurable width, starting at the
                               * least significant bit. The mask scales the range of the random drawn number (PRN) of the model,
                               * here defined as @link drawnRandomNumber @endlink. used as a scale for the random values. */
    //short thresholdMaskBits; //!< TM, or the number of bits for the random number. Use this to generate the thresholdPRN mask;

    //@TODO : Replace short with uint16_t for consistancy.
    //small
    short largestRandomValue;
    short lambda; //!< leak weight - \f$𝜆\f$ Leak tuning parameter - the leak rate applied to the current leak function.
    short int resetMode; //!< Reset mode selection. Valid options are 0,1,2 . Gamma or resetMode 𝛾
    volt_type resetVoltage; //!< Reset voltage for reset params, \f$R\f$.
    short sigmaVR; //!< reset voltage - reset voltage sign
    short encodedResetVoltage; //!< encoded reset voltage - VR.
    short omega; //!<temporary leak direction variable


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
	 /**@} */




	/**
	 * modelState is a future feature - I will be moving the state data of neruons into 
	 * structs that 
	 */
	void *modelState;
	char* neuronTypeDesc; //!< a debug tool, contains a text desc of the neuron.

	neuronTypes neuronType;
	


}neuronState;


#endif //NEMO_NEURON_H
