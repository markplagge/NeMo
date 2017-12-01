#include <stdio.h>
#include <stdlib.h>
#include <simclist.h>

#define BUFLEN  20

int main() {
    list_t l;
    unsigned int i;
    char buf[BUFLEN];

    list_init(&l);
    list_attributes_copy(&l, list_meter_int32_t, 1);
    list_attributes_comparator(&l, list_comparator_int32_t);

    while (fgets(buf, BUFLEN, stdin) != NULL) {
        i = atoi(buf);
        list_append(&l, &i);
    }
    
    list_sort(&l, 1);

    return 0;
}
