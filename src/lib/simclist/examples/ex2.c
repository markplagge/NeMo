#include <stdio.h>

#include <simclist.h>

int main() {
    int val;
    list_t l;

    list_init(&l);

    /* request to store copies, and provide the metric function */
    list_attributes_copy(&l, list_meter_int32_t, 1);

    printf("Give numbers. Terminate with one negative.\n");
    scanf("%d", &val);
    while (val > 0) {
        list_append(&l, &val);
        scanf("%d", &val);
    } 

    /* setting the comparator, so the list can sort, find the min, max etc */
    list_attributes_comparator(&l, list_comparator_int32_t);
    list_sort(&l, -1);      /* sorting the list in descending (-1) order */
    
    /* printing out the result */
    printf("Sorted values:\n");
    
    list_iterator_start(&l);        /* starting an iteration "session" */
    while (list_iterator_hasnext(&l)) { /* tell whether more values available */
        printf("%d\n", *(int *)list_iterator_next(&l));     /* get the next value */
    }
    list_iterator_stop(&l);         /* starting the iteration "session" */
    
    list_destroy(&l);

    return 0;
}

