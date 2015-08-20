//
//  clMapping.h
//  ROSS_TOP
//  Custom Linear Mapping functions using bitfields.
//  Created by Mark Plagge on 8/19/15.
//
//

#ifndef __ROSS_TOP__clMapping__
#define __ROSS_TOP__clMapping__

#include <stdio.h>
#include "ross.h"
#include "assist.h"



/** clMapping - custom struct / bit based mapping. Assumes one core per PE */
tw_peid clMapper(tw_lpid gid);
tw_lpid clLpTypeMapper(tw_lpid gid);
tw_lpid clgetSynapseFromAxon(tw_lpid gid) ;
tw_lpid clGetSynapseFromSynapse(tw_lpid gid) ;
tw_lpid clGetNeuronFromSynapse(tw_lpid gid) ;
tw_lpid clGetAxonFromNeuron(id_type core, id_type local);
tw_lpid clLocalFromGlobal(tw_lpid gid);
//okay - custom mapping:
void clMap();




#endif /* defined(__ROSS_TOP__clMapping__) */
