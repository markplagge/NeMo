#include <stdio.h>


void printlist(list_t *l) {
    int i;

    for (i = 0; i < list_size(l); i++) {
        printf ("> %d\n", *(int *)list_get_at(l, i));
    }
}

