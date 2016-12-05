#include "PSkipList.h"
#include "ThreadHelper.h"

#include <stdio.h>

void PSkipList_init(struct PSkipList *slist){
	int index;

	PLinkedList_init(&slist->towers[0]);
	for (index = 1; index < TOWER_HEIGHT; index++){
		PLinkedList_init(&slist->towers[index]);
		slist->towers[index-1].head->down = slist->towers[index].head;
	}
}

void PSkipList_fini(struct PSkipList *slist){
	int index;
	for (index = 0; index < TOWER_HEIGHT; index++)
		PLinkedList_fini(&slist->towers[index]);
}

void PSkipList_insert(struct PSkipList *slist, int key, int value){
	int index;
	struct PLinkedListNode *prevs[TOWER_HEIGHT] = {};
	struct PLinkedListNode *searches[TOWER_HEIGHT] = {};

	searches[0] = PLinkedList_searchClosestNodeFromNode(&slist->towers[0], slist->towers[0].head, key, &prevs[0]);
	int found = searches[0] ? searches[0]->key == key : 0;

	for (index = 1; index < TOWER_HEIGHT; index++){
		if (!found)
			searches[index] = PLinkedList_searchClosestNodeFromNode(&slist->towers[index], prevs[index-1]->down, key, &prevs[index]);
		else
			searches[index] = searches[index-1] -> down;
		if (searches[index] && searches[index]->key == key){
			found++;
		}
	}

	if (found) {
		//Update all nodes with a new walue
		for (index = TOWER_HEIGHT - 1; found > 0; index--, found--){
			__atomic_store_n(&searches[index]->value, value, __ATOMIC_RELAXED);
		}
	}else{
		//Insert new values
		//We should always added it to at least one list
		struct PLinkedListNode *prevInsertedNode = NULL;
		for (index = TOWER_HEIGHT - 1; index >= 0; index--){
			struct PLinkedListNode *insertedNode = PLinkedList_tryInsertAfter(prevs[index], searches[index], key, value);
			if (!insertedNode){
				insertedNode = PLinkedList_insert(&slist->towers[index], key, value);
			}
			insertedNode->down = prevInsertedNode;
			prevInsertedNode = insertedNode;
			if (thread_randbit()) { return; }
		}
	}
}

int PSkipList_search(struct PSkipList *slist, int key){
	int index;
	struct PLinkedListNode *prevNode = slist->towers[0].head;
	struct PLinkedListNode *foundNode;

	for (index = 0; index < TOWER_HEIGHT; index++){
		if (prevNode != NULL)
			foundNode = PLinkedList_searchClosestNodeFromNode(&slist->towers[index], prevNode, key, &prevNode);
		else
			foundNode = PLinkedList_searchClosestNode(&slist->towers[index], key, &prevNode);
		if (foundNode && foundNode->key == key) return foundNode->value;
		prevNode = prevNode -> down;
	}
	return -1;
}

void PSkipList_delete(struct PSkipList *slist, int key){
	int index, found = 0;
	struct PLinkedListNode *prevs[TOWER_HEIGHT] = {};
	struct PLinkedListNode *searches[TOWER_HEIGHT] = {};

	searches[0] = PLinkedList_searchClosestNode(&slist->towers[0], key, &prevs[0]);
	if(searches[0] && searches[0]->key == key){
		struct PLinkedListNode *nextNode;
		do{
			nextNode = searches[0] -> next;
		}
		while(!__sync_bool_compare_and_swap(&(searches[0] -> next), nextNode, (long)nextNode | 1));
		found = 1;
	}

	for (index = 1; index < TOWER_HEIGHT; index++){
		if (!found){
			if (prevs[index-1]->down)
				searches[index] = PLinkedList_searchClosestNodeFromNode(&slist->towers[index], prevs[index-1]->down, key, &prevs[index]);
			else
				searches[index] = PLinkedList_searchClosestNode(&slist->towers[index], key, &prevs[index]);
		}else{
			searches[index] = searches[index-1] -> down;
		}
		if (searches[index] && searches[index]->key == key){
			struct PLinkedListNode *nextNode;
			do{
				nextNode = searches[index] -> next;
			}
			while(!__sync_bool_compare_and_swap(&(searches[index] -> next), nextNode, (long)nextNode | 1));
			found++;
		}
	}
}


void print_node(struct PLinkedListNode *node){
	printf("[%d-%d]->", node->key, node->value);
}

void PSkipList_debugprint(struct PSkipList *slist){
	int index;
	for (index = 0; index < TOWER_HEIGHT; index++){
		PLinkedList_traverse(&slist->towers[index], print_node); printf("\n");
	}
}
