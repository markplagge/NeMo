//
// Created by Mark Plagge on 2019-09-16.
//





#ifndef SUPERNEMO_MAPPING_H
#define SUPERNEMO_MAPPING_H
#include <ross.h>
#include "include/globals.h"
/**
 * @brief      Gets the pe from gid.
 *
 * @param[in]  gid   The gid
 *
 * @return     The pe from gid.
 */
tw_peid get_pe_from_gid(tw_lpid gid);

/**
 * @brief      Gets the core from gid.
 *
 * @param[in]  gid   The gid
 *
 * @return     The core from gid.
 */
nemo_id_type get_core_from_gid(tw_lpid gid);

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
nemo_id_type get_local_from_gid(tw_lpid gid);

#endif //SUPERNEMO_MAPPING_H
