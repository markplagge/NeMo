//
// Created by Mark Plagge on 11/30/17.
//

#ifndef SUPERNEMO_MORTON_H
#define SUPERNEMO_MORTON_H


/* unsigned long long x,y Morton Z code encoding/decoding/manipulation */

#include <limits.h>
#include <stdint.h>

#define EXLOW(x) (uint32_t) (x)
#define EXHIGH(x) (uint32_t) (x >> 32)



uint64_t combine(unsigned int high, unsigned int low);

#endif //SUPERNEMO_MORTON_H
