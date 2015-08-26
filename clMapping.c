////
////  clMapping.c
////  ROSS_TOP
////
////  Created by Mark Plagge on 8/19/15.
////
////
//
//#include "clMapping.h"
//
//
//
//tw_peid clMapper(tw_lpid gid) {
//
//    GlobalID g;
//    g.raw = gid;
//    int coreOff =  CORES_IN_SIM /(tw_nnodes() * g_tw_npe);
//
//    return (tw_peid) g.core % coreOff;
//
//}
//
//unsigned long long initGIDForPE() {
//    long myPE;
//    MPI_Comm_rank(MPI_COMM_WORLD, &myPE);
//    static long long currentLP = 0;
//    static long long currentCore = 0;
//
//
//    //replace this with TW code:
//    GlobalID g;
//    int coresPerPE = CORES_IN_SIM /(tw_nnodes() * g_tw_npe);
//
//    long coreOffset = (myPE * coresPerPE);
//
//
//
//    if(currentCore < coreOffset){
//        currentCore = coreOffset;
//    }
//    if (currentLP == CORE_SIZE) {
//        currentCore ++;
//        currentLP = 0;
//    }
//    printf("\n\n\n ************************************  PE %i at at core %i - COREOFFSET: %i \n", myPE, currentCore,coreOffset);
//
//    if((currentCore + coreOffset) == coresPerPE + coreOffset){
//
//        return -1; //done.
//    }
//
//    if(currentLP < AXONS_IN_CORE){
//        g.atype = AXON;
//        g.local = currentLP;
//    } else if(currentLP < AXONS_IN_CORE + SYNAPSES_IN_CORE) {
//        g.atype = SYNAPSE;
//        g.local = currentLP - AXONS_IN_CORE;
//    }
//    else{
//        g.atype = NEURON;
//        g.local = currentLP - (SYNAPSES_IN_CORE + AXONS_IN_CORE);
//    }
//    currentLP ++;
//    g.core = currentCore;
//    return g.raw;
//}
//
//
//void clMap(){
//
//    tw_pe *pe;
//    if(CORES_IN_SIM % (tw_nnodes() * g_tw_npe)) //even number of cores per PE.
//        tw_error(TW_LOC, "Must run at least one core per PE");
//    int currentLocal = 0;
//    int currentCore = -1;
//    int ctype = -1;
//    unsigned int nlp_per_pe = CORE_SIZE;
//    int lpid;
//    int kpid;
//    int i;
//    int j;
//    int nlp_per_kp = (int)ceil((double)g_tw_nlp / (double) g_tw_nkp);
//    int nkp_per_pe = g_tw_nkp;
//    if (!nlp_per_kp) {
//        tw_error(TW_LOC, "Not enough KPs defined: %d", g_tw_nkp);
//    }
//
//    GlobalID globStr;
//
//    globStr.raw = 0;
//    globStr.core = (g_tw_mynode * (CORES_IN_SIM / (g_tw_npe*tw_nnodes())));
//    g_tw_mynode = globStr.raw;
//
//#if VERIFY_MAPPING
//    char * coreVals = calloc(9090, sizeof(char));
//    int incro;
//    printf("NODE %ld: nlp %lld, offset %lld\n", g_tw_mynode, g_tw_nlp, g_tw_lp_offset);
//#endif
//
//
//    //We loop through each core first:
//    unsigned long coreID = -1;
//
//
//    for (kpid = 0, lpid = 0, pe = NULL; (pe = tw_pe_next(pe)); )
//    {
//#if VERIFY_MAPPING
//        printf("\tPE %lu\n", pe->id);
//#endif
//
//        for (i = 0; i < nkp_per_pe; i++, kpid++)
//        {
//            tw_kp_onpe(kpid, pe);
//
//#if VERIFY_MAPPING
//            printf("\t\tKP %d", kpid);
//#endif
//
//
//
//            for (j = 0; j < nlp_per_kp && lpid < g_tw_nlp; j++, lpid++)
//            {
//                globStr.raw = initGIDForPE();
//                coreID = globStr.core;
//                //tw_lp_onpe(lpid, pe, localToGlobal(g_tw_lp_offset+lpid));
//                //tw_lp_onpe(lpid, pe, globStr.raw);
//                tw_lp_onpe(lpid, pe, globStr.raw);
//                tw_lp_onkp(g_tw_lp[lpid], g_tw_kp[kpid]);
//
//#if VERIFY_MAPPING
//
//                    GlobalID gvb;
//                    gvb.raw = globStr.raw;
//                    if(currentCore != gvb.core){
//                       // printf("\nCore: %i\n", gvb.core);
//                        sprintf(coreVals, "%s\nCore:%i\n",coreVals, gvb.core);
//                        currentCore = gvb.core;
//                    }
//                    if(ctype != gvb.atype || (gvb.local == AXONS_IN_CORE - 1 || gvb.local == SYNAPSES_IN_CORE -1 )){
//                        char * msg;
//                        switch (gvb.atype) {
//                            case AXON:
//                                msg = "AXON";
//                                break;
//                            case SYNAPSE:
//                                msg = "SYNAPSE";
//                                break;
//                            default:
//                                msg = "NEURON";
//                                break;
//                        }
//
//                        sprintf(coreVals,"%s%s,%i\n",coreVals,msg,gvb.local);
//                        ctype = gvb.atype;
//                    }
//                   // gidg = initGIDForPE();
//
//
//                if (0 == j % 20) {
//                    printf("\n\t\t\t");
//                }
//                printf("%lld ", lpid + g_tw_lp_offset);
//#endif
//            }
//
//
//        }
//    }
//
//    if (!g_tw_lp[g_tw_nlp - 1]) {
//        tw_error(TW_LOC, "Not all LPs defined! (g_tw_nlp=%d)", g_tw_nlp);
//    }
//#if VERIFY_MAPPING
//    printf("\n");
//    printf("%s\n", coreVals);
//#endif
//   // if (g_tw_lp[g_tw_nlp - 1]->gid != g_tw_lp_offset + g_tw_nlp - 1) {
//   //     tw_error(TW_LOC, "LPs not sequentially enumerated!");
//   // }
//}
//tw_lpid clLpTypeMapper(tw_lpid gid){
//    GlobalID g;
//    g.raw = gid;
//    return g.atype;
//}
//
////Model specific mapping functions:
////An Axon will want the next synapse - the synapse it normally talks to.
//
//tw_lpid clgetSynapseFromAxon(tw_lpid gid) {
//    //Axons talk to the next row of synapses. there are NERUONS_IN_CORE synapses in a core per row.
//
//    GlobalID axID;
//    axID.raw = gid;
//    GlobalID syID;
//    syID.core = axID.core;
//    syID.atype = SYNAPSE;
//    syID.local = ((NEURONS_IN_CORE) * axID.local) ;
//    return syID.raw;
//}
//
//tw_lpid clGetSynapseFromSynapse(tw_lpid gid) {
//
//    GlobalID opID;
//    opID.raw = gid;
//    opID.local ++;
//    if (opID.local > NEURONS_IN_CORE)
//        return 0;
//    return opID.raw;
//}
//
//tw_lpid clGetNeuronFromSynapse(tw_lpid gid){
//    GlobalID opID;
//    opID.raw = gid;
//
//    GlobalID nID;
//    nID.raw = 0;
//    nID.atype = NEURON;
//    nID.core = opID.core;
//    nID.local = (opID.local % NEURONS_IN_CORE);
//    return nID.raw;
//}
//
//tw_lpid clGetAxonFromNeuron(id_type core, id_type local){
//    GlobalID aID;
//    aID.atype=AXON;
//    aID.core = core;
//    aID.local = local;
//    return aID.raw;
//
//}
//tw_lp * clLocalFromGlobal(tw_lpid gid) {
//    GlobalID g;
//    g.raw = gid;
//    unsigned long loc = g.local;
//
//    if(g.atype == SYNAPSE) {
//        loc += AXONS_IN_CORE;
//    }
//    else if(g.atype == NEURON) {
//        loc += SYNAPSES_IN_CORE + AXONS_IN_CORE;
//    }
//    return g_tw_lp[loc];
//}
