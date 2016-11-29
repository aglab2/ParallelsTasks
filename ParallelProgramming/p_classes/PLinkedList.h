#ifndef __PLINKEDLIST_H_
#define __PLINKEDLIST_H_

#include <urcu.h>

struct PLinkedList{
	struct PLinkedListNode *head;
};

struct PLinkedListNode{
	struct rcu_head rcu;
	struct PLinkedListNode *next;
	struct PLinkedListNode *down;
	int key;
	int value;
};

void PLinkedList_init(struct PLinkedList *list);
void PLinkedList_fini(struct PLinkedList *list);
struct PLinkedListNode *PLinkedList_insert(struct PLinkedList *list, int key, int value);
struct PLinkedListNode *PLinkedList_insertAfter(struct PLinkedListNode *cur, int key, int value);
struct PLinkedListNode *PLinkedList_tryInsertAfter(struct PLinkedListNode *cur, struct PLinkedListNode *next, int key, int value);

int PLinkedList_search(struct PLinkedList *list, int key);
struct PLinkedListNode* PLinkedList_searchClosestNode(struct PLinkedList *list, int key, struct PLinkedListNode ** prev);
struct PLinkedListNode* PLinkedList_searchClosestNodeFromNode(struct PLinkedList *list, struct PLinkedListNode *node, int key, struct PLinkedListNode ** prev);

void PLinkedList_delete(struct PLinkedList *list, int key);

void PLinkedList_traverse(struct PLinkedList *list, void (*handler)(struct PLinkedListNode *node));

#endif
