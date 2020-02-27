#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "../simclist.h"

#include "printlist.c"

#define N       250

int main() {
    list_t l;
    int i, j, newpos, els[N];

    srandom(time(NULL));
    list_init(&l);

    list_attributes_copy(&l, list_meter_int32_t, 1);
    list_attributes_comparator(&l, list_comparator_int32_t);

    /* insert N zeros */
    printf("Test 1: head insertions ... ");
    fflush(stdout);
    j = 0;
    for (i = 0; i < N; i++) {
        list_insert_at(&l, &j, 0);
        assert((int)list_size(&l) == i+1);
    }
    //list_clear(&l);
    printf("passed.\n");

    /* generate an unsorted list of values from 0 to N */
    printf("Test 2: random insertions and deletions ... ");
    fflush(stdout);
    els[0] = 0;
    for (i = 1; i < N; i++) {
        els[i] = i;
        newpos = random() % (i + 1);
        j = els[newpos];
        els[newpos] = i;
        els[i] = j;
    }

    for (i = 0; i < N; i++) {
        list_insert_at(&l, & els[i], els[i]);
        assert(*(int *)list_get_at(&l, els[i]) == els[i]);
        assert(*(int *)list_get_at(&l, els[i]+1) == 0);
        list_delete_range(&l, els[i]+1, els[i]+1);
    }
    printf("passed.\n");

    printf("Test 3: positioning (The Revenge) ... ");
    fflush(stdout);
    for (i = 0; i < N; i++) {
        assert(*(int *)list_get_at(&l, i) == i);
    }
    printf("passed.\n");


    printf("Test 4: sorting ... ");
    fflush(stdout);
    assert(list_sort(&l, -1) == 0);
    for (i = 0; i < N; i++) {
        assert(*(int *)list_get_at(&l, i) == N-1-i);
    }
    printf("passed.\n");

    list_destroy(&l);

    printf("==========\nAll tests passed.\n");

    return 0;
}

