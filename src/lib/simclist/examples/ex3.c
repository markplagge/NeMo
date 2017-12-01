#include <stdio.h>

#include <simclist.h>

typedef struct {
	int x, y;
} point2D;

typedef struct {
	point2D a, b, c, d;
} rectangle;	/* custom data type to store in list */

/* this function returns the size of elements */
size_t mymeter(const void *el) {
	/* every element has the constant size of a rectangle structure */
	return sizeof(rectangle);
}

/*
 * compare rectangles by area
 * 
 * this function compares two elements:
 * <0: a greater than b
 * 0: a equivalent to b
 * >0: b greater than a
 */
int mycomparator(const void *a, const void *b) {
	/* compare areas */
	const rectangle *A = (rectangle *) a;
	const rectangle *B = (rectangle *) b;
	unsigned int    areaA, areaB;
	areaA = ((A->c.y - A->b.y) * (A->b.x - A->a.x));
	areaB = ((B->c.y - B->b.y) * (B->b.x - B->a.x));
	return (areaA < areaB) - (areaA > areaB);
}

int main() {
	rectangle rect;
	list_t l;

	list_init(&l);

	/* setting the custom spanning function */
	list_attributes_copy(&l, mymeter, 1);

	/* acquire rectangles and insert in list ... */

	/* setting the custom area comparator */
	list_attributes_comparator(&l, mycomparator);
	list_sort(&l, -1);	/* sorting by area (descending) */

	/* [display list ...] */

    list_destroy(&l);

	return 0;
}

