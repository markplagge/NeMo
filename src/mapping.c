//
// Created by Mark Plagge on 5/25/16.
//

#include "mapping.h"

/**
 * @brief      returns the core offset for global->local conversions. 
 * For example, given a core size of 512, and a GID of 513:
 * The offset is (1 * 512)
 * Or if the core size is 512 and the gid is 22, the offset is 0.
 * This is used for calculating core-wise local values.
 *
 * @param[in]  gid   The gid
 *
 * @return     { description_of_the_return_value }
 */
tw_lpid coreOffset(tw_lpid gid){

	return (getCoreFromGID(gid) * CORE_SIZE);
}

tw_lpid getSynapseGIDFromCore(id_type core){
	tw_lpid offset = CORE_SIZE * core;
	return offset + AXONS_IN_CORE;
}
/**
 * NeMo is laid out in a grid fashon. The first \f$n\f$ lps are axons, then
 * there is one synapse lp, then there are \f$n\f$ neurons. 
 */
tw_lpid lpTypeMapper(tw_lpid gid){
	//id_type core = getCoreFromGID(gid); //get the coreID

	id_type local = getLocalFromGID(gid); //get the local ID

	if (local < AXONS_IN_CORE){
		return AXON;
	}
	else if(local - AXONS_IN_CORE == 0){
		return SYNAPSE;
	}
	else{

		return NEURON;
	}

	
}	


//tw_peid getPEFromGID(tw_lpid gid);


id_type getCoreFromGID(tw_lpid gid){
	return (gid / CORE_SIZE);
}

/**
 * Assumes linear mapping of GIDs.
 *
 */
id_type getLocalFromGID(tw_lpid gid){
	return (gid - coreOffset(gid));

}


tw_lpid getGIDFromLocalIDs(id_type core, id_type coreLocal){

	return (core * CORE_SIZE) + coreLocal;
}


tw_lpid getNeuronGlobal(id_type core, id_type neuronID){
	id_type coreLocal = AXONS_IN_CORE + SYNAPSES_IN_CORE + neuronID;
	return getGIDFromLocalIDs(core, coreLocal);
}

/** Assumes that Axons start at GID 0 in a core */
tw_lpid getAxonGlobal(id_type core, id_type axonID){
	return getGIDFromLocalIDs(core, axonID);
}


tw_lpid getSynapseGlobal(id_type core, id_type synapseID){
	id_type coreLocal = AXONS_IN_CORE ;
	return getGIDFromLocalIDs(core, coreLocal);
}
id_type getAxonLocal(tw_lpid gid){
	return getLocalFromGID(gid); //axons start at zero.
}
/** Linear Mapping function */
tw_peid getPEFromGID(tw_lpid gid){
	return (tw_peid) gid / g_tw_nlp;
}

tw_lpid getSynapseFromAxon(tw_lpid axon_id){
	id_type core = getCoreFromGID(axon_id);
	tw_lpid  synGID = getSynapseGIDFromCore(core);
	return synGID;

}
/**
 * Note: this function assumes that it receives a neuron's GID - otherwise it will not return a valid value.
 */
id_type getNeuronLocalFromGID(tw_lpid gid){

	//get the core-wise local value:
	id_type local = getLocalFromGID(gid);

	//Neurons should start at GID of (AXONS_IN_CORE + SYNAPSES_IN_CORE), so
	//neuron 0 in core 0 should have a gid of 512 if 512 axons in core and 1 synapse (super synapse).
	//because GIDs start at 0.


	//then return the neuron local ID. Assuming super synapse configuration of 1 synapse per core:
	return local - AXONS_IN_CORE - 1; /** @todo fix types here for potential large core sizes. */
	//return local - (AXONS_IN_CORE + SYNAPSES_IN_CORE) ;

}