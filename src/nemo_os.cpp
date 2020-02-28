//
// Created by Mark Plagge on 2/27/20.
//

/** FCFS NeMo Initialization System */


#include <iostream>

#include <ross.h>
#include "nemo_os_system.h"
#include "neuro/fcfs_core.h"

//#include "fcfs_logic_system.h"
//#include "neruo/fcfs_core.h"
bool DO_RANDOM_PROCESSES = true;

const tw_optdef app_opt[] = {
        TWOPT_GROUP("Simulated Process Mode"),
        TWOPT_FLAG("rproc", DO_RANDOM_PROCESSES,"Randomized Processes?"),

        TWOPT_END()
};


int nemo_os_main(int argc, char *argv[]){





    return 0;
}
