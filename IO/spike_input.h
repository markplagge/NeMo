//
//  spike_input.h
//  ROSS_TOP
//
//  Created by Mark Plagge on 4/5/16.
//
//

#ifndef spike_input_h
#define spike_input_h

#include <stdio.h>


/** struct that defines mapping - so that files can be split. */
typedef struct IOMapInfo {
    long cores_in_sim;
    long cores_per_pe;
    long neurons_in_core;
}ioMapInfo;




#endif /* spike_input_h */
