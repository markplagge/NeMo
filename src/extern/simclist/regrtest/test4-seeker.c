#include <stdio.h>
#include <stdlib.h>
#include <time.h>   /* for srandom() */
#include "../simclist.h"

#define N       100000
#define LEN     12

struct foo_s {
    int a, b;
    char c[LEN];
};

int seeker(const void  *el, const void *indicator) {
    if (((struct foo_s *)el)->a == *(int *)indicator)
        return 1;
    return 0;
}

size_t mymeter(const void *el) {
    return (sizeof(struct foo_s));
}

int main() {
    list_t l;
    struct foo_s el;
    int i, j;

    list_init(& l);
    list_attributes_seeker(&l, seeker);
    list_attributes_copy(&l, mymeter, 1);

    for (i = 0; i < N; i++) {
        el.a = i;
        el.b = 3*i;
        snprintf(el.c, LEN, "%d-%d", el.a, el.b);
        list_insert_at(& l, & el, i);
    }

    srandom(time(NULL));

    for (i = 0; i < N/4; i++) {
        j = random() % list_size(&l);
        el = *(struct foo_s *)list_seek(&l, &j);
        if (el.a != j) {
            printf("KO: %d retrieved %d\n", j, el.a);
            return 1;
        }
    }

    list_destroy(& l);
    return 0;
}
