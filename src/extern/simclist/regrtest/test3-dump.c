#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include "../simclist.h"

#define N       123

#define hton64(x)       (\
        htons(1) == 1 ?                                         \
            (uint64_t)x      /* big endian */                                 \
        :       /* little endian */                              \
        ((uint64_t)((((uint64_t)(x) & 0xff00000000000000ULL) >> 56) | \
            (((uint64_t)(x) & 0x00ff000000000000ULL) >> 40) | \
            (((uint64_t)(x) & 0x0000ff0000000000ULL) >> 24) | \
            (((uint64_t)(x) & 0x000000ff00000000ULL) >>  8) | \
            (((uint64_t)(x) & 0x00000000ff000000ULL) <<  8) | \
            (((uint64_t)(x) & 0x0000000000ff0000ULL) << 24) | \
            (((uint64_t)(x) & 0x000000000000ff00ULL) << 40) | \
            (((uint64_t)(x) & 0x00000000000000ffULL) << 56)))   \
        )


size_t meter(const void *el) {
    return sizeof(unsigned long long int);
}

void *elserial(const void *el, uint32_t *len) {
    unsigned long long *x;

    *len = sizeof(unsigned long long);
    x = malloc(*len);
    *x = hton64(*(unsigned long long *)el);

    return x;
}

int main() {
    list_t l;
    unsigned long long int data;

    list_init(& l);
    list_attributes_copy(&l, meter, 1);
    list_attributes_serializer(&l, elserial);

    for (data = 1; data < N; data++) {
        list_append(& l, & data);
    }
    if (list_dump_file(&l, "mylistdump.simc") == 0 && errno != 0) {
        printf("fuck off\n");
    }

    list_destroy(& l);

    return 0;
}

