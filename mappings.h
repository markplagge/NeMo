//
// Created by mplagge on 5/1/15.
//



#ifndef ROSS_TOP_MAPPINGS_H
#define ROSS_TOP_MAPPINGS_H
#include <stdio.h>
#include<assert.h>
#include"ross.h"
#include"assist.h"

/** LOC(a) -- bitwise local id getter from a tw_lpid. */
#define LOC(a) ((regid_t)a) & 0xFFFFFFFF
/** CORE(a) -- a bitwise core id getter from a tw_lpid */
#define CORE(a) ((regid_t)(((tw_lpid)(a) >> 32) & 0xFFFFFFFF))


tw_peid mapping(tw_lpid gid);
void getLocalIDs(tw_lpid global, regid_t* core, regid_t* local);
tw_lpid globalID(regid_t core, regid_t local) ;
tw_lp* mapping_to_local(tw_lpid global);
tw_peid mapping(tw_lpid gid);
void initial_mapping(void) ;

void initMapVars(int nInCore,int sInCore,int cpe);

#endif //ROSS_TOP_MAPPINGS_H
