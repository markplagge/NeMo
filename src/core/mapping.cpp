//
// Created by Mark Plagge on 2019-09-16.
//

#include "mapping.h"

nemo_id_type get_core_from_gid(tw_lpid gid){
    // Assuming a linear map for now
    // since GIDs are 0-n, the gid will be == to the core#.
    return (nemo_id_type) gid;

}

tw_lpid lpTypeMapper(tw_lp gid){
    return 0; // all lps are cores right now
}

tw_peid nemo_map_linear(tw_lpid gid){

    return (tw_peid)( gid/g_tw_nlp);
}