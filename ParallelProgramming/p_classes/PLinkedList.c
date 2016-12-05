#include "PLinkedList.h"
#include "ThreadHelper.h"

#include <stdlib.h>
#include <unistd.h>

//#include <urcu.h>

#define CAS(ptr, old, new) __sync_bool_compare_and_swap(ptr, old, new)//(rcu_cmpxchg_pointer(ptr, old, new) == old)

void PLinkedList_init(struct PLinkedList *list){
	list->head = (struct PLinkedListNode *) malloc(sizeof(struct PLinkedListNode));
	list->head->key = -666;
	list->head->next = NULL;
	list->head->value = -666;
}

void PLinkedList_fini(struct PLinkedList *list){
	struct PLinkedListNode *curNode = (struct PLinkedListNode *) thread_unmarkPointer(list->head);

	while(curNode != NULL){
		struct PLinkedListNode *next = curNode->next;
		free(curNode);
		curNode = (struct PLinkedListNode *) thread_unmarkPointer(next);
	}
}

//Returns found node, or NULL if not found
//Puts previous node to prev, if node not found, last one
struct PLinkedListNode* PLinkedList_searchClosestNode(struct PLinkedList *list, int key, struct PLinkedListNode ** prev){
	return PLinkedList_searchClosestNodeFromNode(list, list -> head, key, prev);
}

/*
//TODO: Recover me :]
void nodeFreeFunc(struct rcu_head *head)
{
	struct PLinkedListNode *p = container_of(head, struct PLinkedListNode, rcu);
   	free(p);
}
*/

struct PLinkedListNode* PLinkedList_searchClosestNodeFromNode(struct PLinkedList *list, struct PLinkedListNode *node, int key, struct PLinkedListNode ** prev){
	struct PLinkedListNode *curNode;
	struct PLinkedListNode *prevNode;

	prevNode = node;
	curNode = node->next;
	if (thread_isMarked(curNode)){
		return PLinkedList_searchClosestNodeFromNode(list, list->head, key, prev);
	}
	for (/*to solve ABA, moved this up*/; curNode != NULL; curNode = (struct PLinkedListNode *) thread_unmarkPointer(curNode->next)){
		if (thread_isMarked(curNode)){ //If we appeared to be marked, that is very bad
			return PLinkedList_searchClosestNodeFromNode(list, list->head, key, prev);
		}
		struct PLinkedListNode *nextNode = curNode->next;
		if (thread_isMarked(nextNode)){//Pointer to next element is marked as deleted -- delete element then
			struct PLinkedListNode *clearNextNode = (struct PLinkedListNode *) thread_unmarkPointer(nextNode);
			CAS(&(prevNode->next), curNode, clearNextNode); //Try to clear element
			//And skip it if is logically deleted
			continue;
		}

		if (prevNode != NULL && curNode->key >= key) {
			if (prev) *prev = prevNode;
			return curNode;
		}
		prevNode = curNode;
	}
	if (prev) *prev = prevNode;
	return NULL;
}

void PLinkedList_delete(struct PLinkedList *list, int key){
	struct PLinkedListNode *prevNode;
	struct PLinkedListNode *curNode;
	struct PLinkedListNode *nextNode;

	do{
		curNode = PLinkedList_searchClosestNode(list, key, &prevNode);
		if (!curNode || curNode->key != key) return;
		nextNode = curNode->next;
	}while(!CAS(&(curNode->next), nextNode, (struct PLinkedListNode *) thread_markPointer(nextNode)));
}

int PLinkedList_search(struct PLinkedList *list, int key){
	struct PLinkedListNode *searchNode = PLinkedList_searchClosestNode(list, key, NULL);
	return (searchNode && searchNode->key == key) ? searchNode->value : -1;
}

struct PLinkedListNode *PLinkedList_insertAfter(struct PLinkedListNode *cur, int key, int value){
	struct PLinkedListNode *newNode = (struct PLinkedListNode *) malloc(sizeof(struct PLinkedListNode));
	newNode->down = NULL;
	newNode->key = key;
	newNode->value = value;

	struct PLinkedListNode *next = NULL;
	do{
		next = cur -> next;
		newNode -> next = next;
	}while (!CAS(&(cur -> next), next, newNode));
	return newNode;
}

struct PLinkedListNode *PLinkedList_tryInsertAfter(struct PLinkedListNode *cur, struct PLinkedListNode *next, int key, int value){
	struct PLinkedListNode *newNode = (struct PLinkedListNode *) malloc(sizeof(struct PLinkedListNode));
	newNode->down = NULL;
	newNode->key = key;
	newNode->value = value;
	newNode->next = next;

	if(CAS(&(cur -> next), next, newNode)){
		return newNode;
	}else{
		free(newNode);
		return NULL;
	}
}

struct PLinkedListNode *PLinkedList_insert(struct PLinkedList *list, int key, int value){
	__label__ retry;
	struct PLinkedListNode *prevNode;
	struct PLinkedListNode *searchNode;
retry: searchNode = PLinkedList_searchClosestNode(list, key, &prevNode);
 	if (searchNode && searchNode->key == key){
		searchNode -> value = value;
		return searchNode;
	}else{
		struct PLinkedListNode *insertedNode = PLinkedList_tryInsertAfter(prevNode, searchNode, key, value);
		if (!insertedNode) goto retry;
		return insertedNode;
	}
}

void PLinkedList_traverse(struct PLinkedList *list, void (*handler)(struct PLinkedListNode *)){
	PLinkedList_search(list, 100);
	struct PLinkedListNode *node;
	for (node = list -> head; node != NULL; node = node -> next){
		handler(node);
	}
}
