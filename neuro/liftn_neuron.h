//
// Created by Mark Plagge on 5/25/16.
//

#ifndef __NEMO_TN_NEURON_H__
#define __NEMO_TN_NEURON_H__

#include <math.h>
#include "../IO/IOStack.h"
#include "../globals.h"
#include "../mapping.h"
#define Vj ns->membranePotential

#ifdef SAVE_MSGS

#endif

#ifdef SAVE_NEURON_STATS

#endif


typedef struct LIF_MODEL {

     tw_stime lastActiveTime;
     tw_stime lastLeakTime;
     tw_lpid outputGID;

     stat_type rcvdMsgCount;
     stat_type SOPSCount;

     volt_type membranePotential;

     thresh_type posThreshold;
     thresh_type negThreshold;

     id_type dendriteLocal;

     random_type drawnRandomNumber;

     id_type myCoreID;
     id_type myLocalID;

     short lambda;

     char sigma_l;          //!< leak sign bit - eqiv. to σ
     unsigned char delayVal;  //!<@todo: Need to fully implement this - this value
                              //!is between 1 and 15, a "delay" of n timesteps of a
                              //!neuron. -- outgoing delay //from BOOTCAMP!

     bool firedLast;
     bool heartbeatOut;
     bool isSelfFiring;

     bool canGenerateSpontaniousSpikes;

     char axonTypes[512];
     char synapticWeight[4];
     bool synapticConnectivity[512];  //!< is there a connection between axon i and
                                      //!neuron j?
     /** stochastic weight mode selection. $b_j^{G_i}$ */
     bool weightSelection[4];
} lif_neuron_state;


/**
 * @brief      True North Forward Event handler
 *
 * @param      s  The tn neuron state
 * @param      CV               flags for message flow
 * @param      messageData      The message data
 * @param      lp               The pointer to a LP
 */
void LIF_forward_event(lif_neuron_state *s, tw_bf *CV, messageData *m, tw_lp *lp);

/**
 * @brief      True North Reverse Event Handler
 *
 * @param      s  The tn neuron state
 * @param      CV               flags for message flow
 * @param      messageData      The message data
 * @param      lp               The pointer to a
 */
void LIF_reverse_event(lif_neuron_state *s, tw_bf *CV, messageData *m, tw_lp *lp);

void LIF_commit(lif_neuron_state *s, tw_bf *cv, messageData *m, tw_lp *lp);

/**
 * @brief      Initialize a TrueNorth neuron
 *
 * @param      s     The TN State
 * @param      lp    The pointer to the LP
 */
void LIF_init(lif_neuron_state *s, tw_lp *lp);

/**
 * @brief      The TN neuron final function
 *
 * @param      s     TN State
 * @param      lp    The pointer to an LP
 */
void LIF_final(lif_neuron_state *s, tw_lp *lp);

/**
 * @brief	This takes a void pointer and returns this neuron's struct.
 * This is used for managing super synapse direct communication functionality.
 */

inline lif_neuron_state *LIF_convert(void *lpstate);

#endif  // NEMO_TN_NEURON_H
