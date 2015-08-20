//
//  clMapping.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 8/19/15.
//
//

#include "clMapping.h"



tw_peid clMapper(tw_lpid gid) {
    
    GlobalID g;
    g.raw = gid;
    return (tw_peid) g.core;
    
}

void clMap(){
    
    tw_pe *pe;
    
    unsigned int nlp_per_pe = CORE_SIZE;
    int lpid;
    int kpid;
    int i;
    int j;
    int nlp_per_kp = (int)ceil((double)g_tw_nlp / (double) g_tw_nkp);
    int nkp_per_pe = g_tw_nkp;
    if (!nlp_per_kp) {
        tw_error(TW_LOC, "Not enough KPs defined: %d", g_tw_nkp);
    }
    
    GlobalID globStr;
    globStr.raw = 0;
    globStr.core =
    
    g_tw_lp_offset = g_tw_mynode;
    
#if VERIFY_MAPPING
    printf("NODE %ld: nlp %lld, offset %lld\n", g_tw_mynode, g_tw_nlp, g_tw_lp_offset);
#endif
    
    for (kpid = 0, lpid = 0, pe = NULL; (pe = tw_pe_next(pe)); )
    {
#if VERIFY_MAPPING
        printf("\tPE %lu\n", pe->id);
#endif
        
        for (i = 0; i < nkp_per_pe; i++, kpid++)
        {
            tw_kp_onpe(kpid, pe);
            
#if VERIFY_MAPPING
            printf("\t\tKP %d", kpid);
#endif
            
            for (j = 0; j < nlp_per_kp && lpid < g_tw_nlp; j++, lpid++)
            {
                tw_lpid offLPID = lpid + g_tw_lp_offset;
                
                if(lpid < AXONS_IN_CORE){
                    globStr.atype = AXON;
                    globStr.local = lpid;
                } else if(lpid < AXONS_IN_CORE + SYNAPSES_IN_CORE) {
                    globStr.atype = SYNAPSE;
                    globStr.local = lpid - AXONS_IN_CORE;
                }
                else{
                    globStr.atype = NEURON;
                    globStr.local = lpid - (SYNAPSES_IN_CORE + AXONS_IN_CORE);
                }
                
                //tw_lp_onpe(lpid, pe, localToGlobal(g_tw_lp_offset+lpid));
                tw_lp_onpe(lpid, pe, globStr.raw);
                tw_lp_onkp(g_tw_lp[lpid], g_tw_kp[kpid]);
                
#if VERIFY_MAPPING
                if (0 == j % 20) {
                    printf("\n\t\t\t");
                }
                printf("%lld ", lpid + g_tw_lp_offset);
#endif
            }
            
#if VERIFY_MAPPING
            printf("\n");
#endif
        }
    }
    
    if (!g_tw_lp[g_tw_nlp - 1]) {
        tw_error(TW_LOC, "Not all LPs defined! (g_tw_nlp=%d)", g_tw_nlp);
    }
    
   // if (g_tw_lp[g_tw_nlp - 1]->gid != g_tw_lp_offset + g_tw_nlp - 1) {
   //     tw_error(TW_LOC, "LPs not sequentially enumerated!");
   // }
}
tw_lpid clLpTypeMapper(tw_lpid gid){
    GlobalID g;
    g.raw = gid;
    return g.atype;
}

//Model specific mapping functions:
//An Axon will want the next synapse - the synapse it normally talks to.

tw_lpid clgetSynapseFromAxon(tw_lpid gid) {
    //Axons talk to the next row of synapses. there are NERUONS_IN_CORE synapses in a core per row.
    
    GlobalID axID;
    GlobalID syID;
    syID.core = axID.core;
    syID.atype = SYNAPSE;
    syID.local = ((NEURONS_IN_CORE) * axID.local) ;
    return syID.raw;
}

tw_lpid clGetSynapseFromSynapse(tw_lpid gid) {
    
    GlobalID opID;
    opID.raw = gid;
    opID.local ++;
    if (opID.local > NEURONS_IN_CORE)
        return 0;
    return opID.raw;
}

tw_lpid clGetNeuronFromSynapse(tw_lpid gid){
    GlobalID opID;
    opID.raw = gid;
    
    GlobalID nID;
    nID.atype = NEURON;
    nID.core = opID.core;
    nID.local = (opID.local % NEURONS_IN_CORE);
    return nID.local;
}

tw_lpid clGetAxonFromNeuron(id_type core, id_type local){
    GlobalID aID;
    aID.atype=AXON;
    aID.core = core;
    aID.local = local;
    return aID.raw;
    
}
tw_lpid clLocalFromGlobal(tw_lpid gid) {
    GlobalID g;
    g.raw = gid;
    unsigned long loc = g.local;
    
    if(g.atype == SYNAPSE) {
        loc += AXONS_IN_CORE;
    }
    else if(g.atype == NEURON) {
        loc += SYNAPSES_IN_CORE + AXONS_IN_CORE;
    }
    return loc;
}
