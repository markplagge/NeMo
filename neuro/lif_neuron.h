//
// Created by Mark Plagge on 5/25/16.
//

#ifndef __NEMO_LIF_NEURON_H__
#define __NEMO_LIF_NEURON_H__



#include "../globals.h"
#include "../mapping.h"
#include "../IO/IOStack.h"
#include <math.h>
#define Vj ns->membranePotential




typedef struct LIF_MODEL
{


     //64
     tw_stime lastActiveTime; /**< last time the neuron fired - used for calculating leak and reverse functions. Should be a whole number (or very close) since big-ticks happen on whole numbers. */
     tw_stime lastLeakTime;/**< Timestamp for leak functions. Should be a mostly whole number, since this happens once per big tick. */
     tw_lpid outputGID; //!< The output GID (axon global ID) of this neuron.

     stat_type rcvdMsgCount; //!<  The number of synaptic messages received.
     stat_type SOPSCount; //!<  A count for SOPS calculation


     //32
     volt_type V_mem; //!< current "voltage" of neuron, \f$V_j(t)\f$. Since this is PDES, \a t is implicit
     volt_type V_in;
     volt_type V_spike;
     volt_type V_last;
     thresh_type V_thresh;

     //16
     id_type dendriteLocal; //!< Local ID of the remote dendrite -- not LPID, but a local axon value (0-i)

     id_type myCoreID; //!< Neuron's coreID
     id_type myLocalID;//!< my local ID - core wise. In a 512 size core, neuron 0 would have a local id of 262,657.


     //small
     int firing_count;
     int refract_length;
     double R_mem;
     double C_mem;
     double Tau;


     unsigned char delayVal; //!<@todo: Need to fully implement this - this value is between 1 and 15, a "delay" of n timesteps of a neuron. -- outgoing delay //from BOOTCAMP!

     bool heartbeatOut;

     char axonTypes[512];
     char synapticWeight[4];
     bool synapticConnectivity[512]; //!< is there a connection between axon i and neuron j?
     /** stochastic weight mode selection. $b_j^{G_i}$ */
     bool weightSelection[4];


}lif_neuron_state





/**
 * @brief      Leaky Integrate and Fire Forward Event handler
 *
 * @param      s                The lif neuron state
 * @param      CV               flags for message flow
 * @param      messageData      The message data
 * @param      lp               The pointer to a LP
 */
void LIF_forward_event (lif_neuron_state *s, tw_bf *CV, messageData *m,
    tw_lp *lp);

/**
 * @brief      Leaky Integrate and Fire Reverse Event Handler
 *
 * @param      s                The lif neuron state
 * @param      CV               flags for message flow
 * @param      messageData      The message data
 * @param      lp               The pointer to a
 */
void LIF_reverse_event (lif_neuron_state *s, tw_bf *CV, messageData *m,
    tw_lp *lp);


void LIF_commit(lif_neuron_state *s, tw_bf * cv, messageData *m, tw_lp *lp);

/**
 * @brief      Initialize a Leaky Integrate and Fire neuron
 *
 * @param      s     The lif neuron state
 * @param      lp    The pointer to the LP
 */
void LIF_init(lif_neuron_state *s, tw_lp *lp);

/**
 * @brief      The Leaky Integrate and Fire neuron final function
 *
 * @param      s     The lif neuron State
 * @param      lp    The pointer to an LP
 */
void LIF_final(lif_neuron_state *s, tw_lp *lp);


/**
 * @brief	This takes a void pointer and returns this neuron's struct.
 * This is used for managing super synapse direct communication functionality.
 */

// inline lif_neuron_state * TN_convert(void * lpstate);


#endif //NEMO_LIF_NEURON_H
