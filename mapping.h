//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_MAPPING_H
#define NEMO_MAPPING_H

#include "globals.h"

/**
 * @brief      lpTypeMapper maps a given GID to a lp type - neuron, synapse, axon, etc
 * Assumes neuromorphic hardware grid laout with \f$NxN\f$ - number of neurons == number of axons.
 * 
 * @param[in]  gid   The gid
 *
 * @return     returns the lp type id value. 
 */
tw_lpid lpTypeMapper(tw_lpid gid);

/**
 * @brief      Gets the pe from gid.
 *
 * @param[in]  gid   The gid
 *
 * @return     The pe from gid.
 */
tw_peid getPEFromGID(tw_lpid gid);

/**
 * @brief      Gets the core from gid.
 *
 * @param[in]  gid   The gid
 *
 * @return     The core from gid.
 */
id_type getCoreFromGID(tw_lpid gid);
/**
 * @brief      Gets the local from gid. Local ID here is on a \f$0-s\f$ scale. \f$s\f$
 * is the size of a core in the simulation. This is different from a local type
 * id. Local type IDs exist to allow reasoning about neurons and axons in a \f$0-n\f$
 * fashon, where \f$n\f$ is the number of neurons or axons in the sim. 
 *
 * @param[in]  gid   The gid
 *
 * @return     The local from gid.
 */
id_type getLocalFromGID(tw_lpid gid);

/**
 * @brief      Gets the neuron local (not corewise) from gid. The first neuron is
 * 0, the second neuron is 1, and so on.
 *
 * @param[in]  gid   The gid
 *
 * @return     The neuron local from gid.
 */
id_type getNeuronLocalFromGID(tw_lpid gid);
/**
 * @brief      Gets the gid from local ids. Local id here is on a \f$0-s\f$ scale,
 * where \f$s\f$ is the total size of a core. This is not a local type based ID.
 * @related getGIDFromLocalIDs
 * @param[in]  core   The core
 * @param[in]  coreLocal  The core-wise local
 *
 * @return     The gid from local ids.
 */
tw_lpid getGIDFromLocalIDs(id_type core, id_type coreLocal);

/**
 * @brief      Gets the neuron global value from a given neuron id. 
 *
 * @param[in]  core      The core
 * @param[in]  neuronID  The neuron id (0-n, where n is the number of neurons in the sim)
 *
 * @return     The neuron global.
 */
tw_lpid getNeuronGlobal(id_type core, id_type neuronID);

/**
 * @brief      Gets the axon global id from a given axon ID.
 *
 * @param[in]  core    The core
 * @param[in]  axonID  The axon id (0-n where n is the number of axons in the sim)
 *
 * @return     The axon global.
 */
tw_lpid getAxonGlobal(id_type core, id_type axonID);

/**
 * @brief      Gets the synapse global id from a given local synapse ID.
 *
 * @param[in]  core       The core ID.
 * @param[in]  synapseID  The synapse id - local grid id: 0-x, 
 * where x is the number of synapses in the sim.
 *
 * @return     The synapse global ID.
 */
tw_lpid getSynapseGlobal(id_type core, id_type synapseID);

/**
 * @brief      Gets the axon local ID based on the gid. This is axon-specific, not core-centric local.
 *
 * @param[in]  gid   The gid
 *
 * @return     The axon local.
 */
id_type getAxonLocal(tw_lpid gid);

/**
 * @brief      Gets the synapse GID from axon.
 *
 * @param[in]  axon_id  The axon identifier
 *
 * @return     The synapse from axon.
 */
tw_lpid getSynapseFromAxon(id_type axon_id);




#endif //NEMO_MAPPING_H
