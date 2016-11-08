#ifndef __BTREE_H_
#define __BTREE_H_

#include <stdint.h>

//-1 is reserved

#define MAX_SIZE 10

typedef struct btree_node *btree;

struct data_t{
	uint64_t key;
	uint64_t value;
};

struct btree_node {
	int is_leaf;
	int size;
	struct data_t data[MAX_SIZE + 1]; //This is made 1 element more to easily process popped element
	btree children[MAX_SIZE + 2];
};

btree btree_create(void);
void btree_destroy(btree t);
uint64_t btree_search(btree t, uint64_t key);
void btree_insert(btree t, struct data_t elem);
void btree_delete(btree t, uint64_t key);

#endif
