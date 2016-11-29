#include "PLinkedList.h"
#include "ThreadHelper.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <assert.h>

#include <urcu.h>

struct thread_data{
	struct PLinkedList *list;
	int num;
};

#define NUM_THREADS 8

void print_node(struct PLinkedListNode *node){
	printf("[%d-%d]->", node->key, node->value);
}

#define TEST_SIZE 100

void *thread_func(void *ptr){
	rcu_register_thread();
	struct thread_data *data = (struct thread_data *) ptr;

	struct PLinkedList *list = data->list;
	int num = data->num;

	int i;

	for (i = 0; i < TEST_SIZE; i++){
		PLinkedList_insert(list, num+i, i*2);
	}
	for (i = 0; i < TEST_SIZE; i++){
		assert(PLinkedList_search(list, num+i) == i*2);
	}
	for (i = 0; i < TEST_SIZE; i++){
		PLinkedList_delete(list, num+i);
	}
	for (i = 0; i < TEST_SIZE; i++){
		assert(PLinkedList_search(list, num+i) == -1);
	}

	rcu_unregister_thread();
	return 0;
}

void test_list(int num_threads){
	struct PLinkedList list;
	int i;

	pthread_t *threads = (pthread_t *) malloc(num_threads*sizeof(pthread_t));
	struct thread_data *data = (struct thread_data *) malloc(num_threads*sizeof(struct thread_data));

	PLinkedList_init(&list);
	for (i = 0; i < num_threads; i++){
		data[i].list = &list;
		data[i].num = 2*TEST_SIZE*i;
		pthread_create(threads+i, NULL, thread_func, data+i);
	}

	for (i = 0; i < num_threads; i++){
		pthread_join(threads[i], NULL);
	}

	PLinkedList_fini(&list);
	free(threads);
	free(data);
}

int main(int argc, char const *argv[]) {
	int num_threads = 0;
	for (num_threads = 1; num_threads < MAX_THREADS; num_threads*=2)
		test_list(num_threads);
	return 0;
}
