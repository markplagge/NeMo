//
// Created by Mark Plagge on 11/30/17.
//

#include "itrlve.h"
uint64_t combine(unsigned int high, unsigned int low) {
return (((uint64_t) high << 32) | ((uint64_t) low));
}