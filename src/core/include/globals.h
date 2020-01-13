//
// Created by Mark Plagge on 2019-09-16.
//





#ifndef SUPERNEMO_GLOBALS_H
#define SUPERNEMO_GLOBALS_H
/**
 * @defgroup global_help Global Helpers.
 * Global helper functions / classes which are used throughout NeMo @{ */

#include <cstdint>
#include <vector>
/** Gives us the BINCOMP (binary comparison) function used for stochastic weight modes.
 * Takes the absolute value of the first value, and compares it to the seocnd. */
template <typename T1, typename T2>
bool bincomp(T1 val1, T2 val2){
    return abs(val1) >= val2;
}
/** Kroniker Delta Function for TrueNorth */
template <typename T1>
constexpr auto dt(T1 x){
    return !(x);
}
/** SGN Function */
template <typename T1>
constexpr auto sgn(T1 x){
    return ((x > 0) - (x < 0));
}

/** @defgroup types Typedef Vars
 * Typedefs to ensure proper types for the neuron parameters/mapping calculations
 * @{  */

typedef int_fast64_t
        nemo_id_type; //!< id type is used for local mapping functions - there should be $n$ of them depending on CORE_SIZE
typedef int32_t nemo_volt_type; //!< volt_type stores voltage values for membrane potential calculations
typedef int64_t nemo_weight_type;//!< seperate type for synaptic weights.
typedef int32_t nemo_thresh_type;//!< Type for weights internal to the neurons.
typedef uint16_t nemo_random_type;//!< Type for random values in neuron models.

typedef uint64_t size_type; //!< size_type holds sizes of the sim - core size, neurons per core, etc.

typedef uint64_t stat_type;
/**@}*/
typedef enum CoreTypes{
    TN,
    LIF
}core_types;

long cores_per_processor = 65535;


#endif //SUPERNEMO_GLOBALS_H
