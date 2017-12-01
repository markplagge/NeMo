#include <stdio.h>

#include <simclist.h>   /* use the SimCList library */


int main() {
    list_t mylist;      /* declare a list */
    int userval;


    list_init(& mylist);    /* initialize the list */

    printf("Insert your number: ");
    scanf("%d", & userval);

    list_append(& mylist, & userval);       /* add an element to the list */
    
    printf("The list now holds %u elements.\n", \
            list_size(& mylist));           /* get the size of the list */

    printf("Your number was: %d\n", \
            * (int*)list_get_at(& mylist, 0));  /* extract the first element of the list */

    list_destroy(&mylist);
    
    return 0;
}

