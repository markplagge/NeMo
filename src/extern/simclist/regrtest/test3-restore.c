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


void *elunserial(const void *el, uint32_t *len) {
    unsigned long long *x;

    *len = sizeof(unsigned long long);
    x = malloc(*len);
    *x = hton64(*(unsigned long long *)el);

    return x;
}

int main() {
    list_t l;
    unsigned long long int data, val;
    unsigned int mem;

    list_init(& l);
    list_attributes_unserializer(&l, elunserial);

    mem = list_restore_file(&l, "mylistdump.simc");
    if (mem == 0 && errno != 0) {
        perror("open");
        printf("fuck off\n");
    } else {
        printf("Restored successfully:\n");
        printf("N els: %u\nmemread: %u\n", list_size(&l), mem);
        for (data = 1; data < N; data++) {
            val = *(unsigned long long *)list_get_at(&l, (unsigned int)data-1);
            if (data != val) {
                printf("Wrong data. Pos %llu, expected %llu, got %llu\n", data-1, data, val);
                return 0;
            }
            printf("%lld ", val);
        }
        printf("\n");
    }

    list_destroy(& l);

    return 0;
}

