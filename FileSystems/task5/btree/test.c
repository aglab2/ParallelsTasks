#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "btree.h"

int main(int argc, char **argv) {
	btree b;
	int i;

	b = btree_create();
	assert(b);

	struct data_t elem;
	elem.key = 42;
	elem.value = 24;

	assert(btree_search(b, 42) == -1);
	btree_insert(b, elem);
	assert(btree_search(b, 42) == 24);
	btree_destroy(b);

	b = btree_create();
	for (i = 0; i < 100; i++) {
		printf("%d\n", i);
		elem.key = i;
		elem.value = i*i;
		assert(btree_search(b, i) == -1);
		printf("\tsearch1\n", i);
		btree_insert(b, elem);
		printf("\tinsert\n", i);
		assert(btree_search(b, i) == i*i);
		printf("\tsearch2\n", i);
	}

	for (i = 0; i < 100; i++) {
		printf("%d\n", i);
		elem.key = i;
		elem.value = i*i*i;
		assert(btree_search(b, i) == i*i);
		printf("\tsearch1\n", i);
		btree_insert(b, elem);
		printf("\tinsert\n", i);
		assert(btree_search(b, i) == i*i*i);
		printf("\tsearch2\n", i);
	}

	btree_destroy(b);
	return 0;
}
