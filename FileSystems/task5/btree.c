#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btree.h"

int array_searchhigh_key(int size, const struct data_t *array, uint64_t key) {
	int low  = -1;
	int high = size;
	int mid  = 0;

	//Do regular binary search over array
	while (low + 1 < high) {
		mid = (low + high) / 2;
		if (array[mid].key == key)
			return mid;
		else if (array[mid].key < key)
			low = mid;
		else
			high = mid;
	}

	return high;
}

btree btree_create(void) {
	btree b = (btree) malloc(sizeof(*b));

	b->is_leaf = 1;
	b->size = 0;

	return b;
}

void btree_destroy(btree b) {
	int i;

	//Recursive tree destruction
	if (!b->is_leaf)
		for (i = 0; i < b->size + 1; i++)
			btree_destroy(b->children[i]);

	free (b);
}

//Returns -1 if element was not found
uint64_t btree_search(btree b, uint64_t key) {
	if (b->size == 0)
		return -1;

	//Recursive tree search over arrays
	int pos = array_searchhigh_key(b->size, b->data, key);

	if (pos < b->size && b->data[pos].key == key)
		return b->data[pos].value;

	if (b->is_leaf)
		return -1;

	return btree_search(b->children[pos], key);
}

//Create right sibling if required, or returns NULL if remake is not required
static btree btree_insert_(btree b, struct data_t elem, struct data_t *median) {
	int pos;
	int mid;

	pos = array_searchhigh_key(b->size, b->data, elem.key);

	//Such key is already inserted -- skip
	if (pos < b->size && b->data[pos].key == elem.key) {
		b->data[pos] = elem;
		return NULL;
	}

	if (b->is_leaf) {
		//Insert new element in list with shift of other elements
		memmove(&b->data[pos + 1], &b->data[pos], sizeof(struct data_t) * (b->size - pos));
		b->data[pos] = elem;
		b->size++;
	} else {
		//Insert into child
		struct data_t child_median;
		btree b_child = btree_insert_(b->children[pos], elem, &child_median);

		//If stuff left, insert in current tree
		if (b_child) {
			//Insert popped element in array with shift of other elements
			//Data from left element
			memmove(&b->data[pos + 1], &b->data[pos], sizeof(struct data_t) * (b->size - pos));
			b->data[pos] = child_median;
			//Node itself
			memmove(&b->children[pos + 2], &b->children[pos + 1], sizeof(struct btree_node) * (b->size - pos));
			b->children[pos + 1] = b_child;
			b->size++;
		}
	}

	if (b->size >= MAX_SIZE) {
		//Search median to split tree
		mid = b->size / 2;
		*median = b->data[mid];

		//Make a new node -- only one, the left one is old
		btree b_right = (btree) malloc(sizeof(struct btree_node));

		b_right->size = b->size - mid - 1;
		b_right->is_leaf = b->is_leaf;

		//memcpy data to right tree
		memmove(b_right->data, &b->data[mid + 1], sizeof(struct data_t) * b_right->size);
		if (!b->is_leaf)
			memmove(b_right->children, &b->children[mid + 1], sizeof(btree) * (b_right->size + 1));

		//And trim the left tree
		b->size = mid;

		return b_right;
	} else {
		return NULL;
	}
}

void btree_insert(btree b, struct data_t elem) {
	struct data_t median;

	btree b_right = btree_insert_(b, elem, &median);

	if (b_right) {
		//We should split root node -- then we should change root to another node
		btree b_left = (btree) malloc(sizeof(*b_left));
		memmove(b_left, b, sizeof(*b));

		//And setup b to be have only 2 children b_left and b_right
		b->size = 1;
		b->is_leaf = 0;
		b->data[0] = median;
		b->children[0] = b_left;
		b->children[1] = b_right;
	}
}

void btree_delete(btree b, uint64_t key) {
	struct data_t elem;
	elem.key = key; elem.value = -1;
	btree_insert(b, elem);
}
