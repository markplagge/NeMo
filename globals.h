//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_GLOBALS_H
#define NEMO_GLOBALS_H
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <limits.h>
#include "ross.h"

/** Typedefs to ensure proper types for the neuron parameters/mapping calculations */

typedef int_fast16_t id_type; //!< id type is used for local mapping functions - there should be $n$ of them depending on @CORE_SIZE
typedef int32_t volt_type; //!< volt_type stores voltage values for membrane potential calculations
typedef int64_t weight_type;//!< seperate type for synaptic weights.
typedef uint32_t thresh_type;//!< Type for weights internal to the neurons.


/*Global Macros */
/** IABS is an integer absolute value function */

//#define IABS(a) (((a) < 0) ? (-a) : (a))

/** Faster version  of IABS (no branching) but needs types. @todo this
 method will be faster on BGQ, but need to make sure that it works properly */
 weight_type IABS(weight_type in){
    //int_fast64_t const mask = in >> sizeof(int_fast64_t) * CHAR_BIT - 1;
    #ifdef HAVE_SIGN_EXTENDING_BITSHIFT
    int const mask = v >> sizeof(int) * CHAR_BIT - 1;
    #else
    int const mask = -((unsigned)v >> sizeof(int) * CHAR_BIT - 1);
    #endif
    return (in ^ mask) - mask;
}
//32bit X86 Assembler IABS:
int iIABS(int vals){
    int IABS(int vals){
        int result;
        asm ("movl  %[valI], %%eax;"
                "cdq;"
                "xor %%edx, %%eax;"
                "sub %%edx, %%eax;"
                "movl %%eax, %[resI];"
        : [resI] "=r" (result)
        : [valI] "r" (vals)
        : "cc","%eax", "%ebx");
        return result;
    }

}
#endif //NEMO_GLOBALS_H
