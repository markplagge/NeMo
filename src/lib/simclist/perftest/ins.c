
#include <stdio.h>
#include <simclist.h>

#define NELS    10000000

int main() {
    list_t l;
    int i;

    list_init(&l);
    for (i = 0; i < NELS; i++) {
        list_append(&l, &i);
    }

    return 0;
}
