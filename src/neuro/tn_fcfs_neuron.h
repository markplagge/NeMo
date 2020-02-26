//
// Created by Mark Plagge on 5/25/16.
//

#ifndef __NEMO_TN_NEURON_H__
#define __NEMO_TN_NEURON_H__

#include <math.h>
#include <assert.h>
#include <search.h>
#include "../IO/IOStack.h"
#include "../globals.h"
#include "../mapping.h"
#include "../dumpi.h"
#include "../tests/tomacs_exp.h"
#include "../nemo_config.h"
#include "../IO/output.h"
#include "./tn_neuron_struct.h"
#define Vj ns->membranePotential

#ifdef NET_IO_DEBUG
#include <stdarg.h>
#endif

/** DUMPI FILE */



void fcfs_create_neuron_encoded_rv(
    id_type coreID, id_type nID, bool synapticConnectivity[NEURONS_IN_CORE],
    short G_i[NEURONS_IN_CORE], short sigma[4], short S[4], bool b[4],
    bool epsilon, short sigma_l, short lambda, bool c, uint32_t alpha,
    uint32_t beta, short TM, short VR, short sigmaVR, short gamma, bool kappa,
    tn_neuron_state *n, int signalDelay, uint64_t destGlobalID,
    int destAxonID);
void fcfs_create_neuron_encoded_rv_non_global(
                                            int coreID, int nID, bool synapticConnectivity[NEURONS_IN_CORE],
                                            short G_i[NEURONS_IN_CORE], short sigma[4], short S[4], bool b[4],
                                            bool epsilon, int sigma_l, int lambda, bool c, int alpha,
                                            int beta, int TM, int VR, int sigmaVR, int gamma, bool kappa,
                                            tn_neuron_state *n, int signalDelay, int destCoreID,
                                            int destAxonID);

/**
 * @brief      True North Forward Event handler
 *
 * @param      s  The tn neuron state
 * @param      CV               flags for message flow
 * @param      messageData      The message data
 * @param      lp               The pointer to a LP
 */
void fcfs_forward_event(tn_neuron_state *s, tw_bf *CV, messageData *m, tw_lp *lp);

/**
 * @brief      True North Reverse Event Handler
 *
 * @param      s  The tn neuron state
 * @param      CV               flags for message flow
 * @param      messageData      The message data
 * @param      lp               The pointer to a
 */
void fcfs_reverse_event(tn_neuron_state *s, tw_bf *CV, messageData *m, tw_lp *lp);

void fcfs_commit(tn_neuron_state *s, tw_bf *cv, messageData *m, tw_lp *lp);

/**
 * @brief      Initialize a TrueNorth neuron
 *
 * @param      s     The TN State
 * @param      lp    The pointer to the LP
 */
void fcfs_init(tn_neuron_state *s, tw_lp *lp);

/**
 * TN_pre_run cleans up the input files after an initialization/
 * @param s
 * @param me
 */
void fcfs_pre_run(tn_neuron_state *s, tw_lp *me);
/**
 * @brief      The TN neuron final function
 *
 * @param      s     TN State
 * @param      lp    The pointer to an LP
 */
void fcfs_final(tn_neuron_state *s, tw_lp *lp);

/**
 * @brief	This takes a void pointer and returns this neuron's struct.
 * This is used for managing super synapse direct communication functionality.
 */

inline fcfs_neuron_state *FCFS_convert(void *lpstate);

size_t fcfs_size(tn_neuron_state *s, tw_lp *lp);
void fcfs_serialize(tn_neuron_state *s, void *buffer, tw_lp *lp);
void fcfs_deserialize(tn_neuron_state *s, void *buffer, tw_lp *lp);


#endif  // NEMO_TN_NEURON_H
