
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <simclist.h>

#define NELS    1000000

size_t szelem(const void *el) {
    return sizeof(int);
}

int main() {
    list_t l;
    int i;

    srandom(time(NULL));

    list_init(&l);
    list_attributes_copy(&l, szelem, 1);

    for (i = 0; i < NELS; i++) {
        list_append(&l, &i);
    }

    for (i = 0; i < 1000; i++) {
        list_get_at(&l, random()%NELS);
    }

    return 0;
}
