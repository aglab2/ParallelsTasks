#ifndef __PSKIPLIST_H_
#define __PSKIPLIST_H_

#include "PLinkedList.h"

#define TOWER_HEIGHT 32

struct PSkipList{
	struct PLinkedList towers[TOWER_HEIGHT];
};

void PSkipList_init(struct PSkipList *slist);
void PSkipList_fini(struct PSkipList *slist);
void PSkipList_insert(struct PSkipList *slist, int key, int value);
int PSkipList_search(struct PSkipList *slist, int key);
void PSkipList_delete(struct PSkipList *slist, int key);

void PSkipList_debugprint(struct PSkipList *slist);

#endif
