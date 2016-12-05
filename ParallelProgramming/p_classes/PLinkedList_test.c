#include "PLinkedList.h"
#include "ThreadHelper.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <sys/time.h>

#include <assert.h>

struct thread_data{
	struct PLinkedList *list;
	int num;
	int test_size;
};

void print_node(struct PLinkedListNode *node){
	printf("[%d-%d]->", node->key, node->value);
}

#define TEST_SIZE 64000

void *thread_func(void *ptr){
	struct thread_data *data = (struct thread_data *) ptr;

	struct PLinkedList *list = data->list;
	int num = data->num;
	int test_size = data->test_size;

	int i;

	for (i = 0; i < test_size; i++){
		PLinkedList_insert(list, num+i, i*2);
	}
	for (i = 0; i < test_size; i++){
		assert(PLinkedList_search(list, num+i) == i*2);
	}
	for (i = 0; i < test_size; i++){
		PLinkedList_delete(list, num+i);
	}
	for (i = 0; i < test_size; i++){
		assert(PLinkedList_search(list, num+i) == -1);
	}

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
		data[i].num = (TEST_SIZE / num_threads)*i;
		data[i].test_size = TEST_SIZE / num_threads;
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
	struct timeval start, end;
	double elapsedTime;

	int num_threads = 0;
	for (num_threads = 1; num_threads <= MAX_THREADS; num_threads*=2){
		gettimeofday(&start, NULL);
		test_list(num_threads);
		gettimeofday(&end, NULL);

		elapsedTime = (end.tv_sec-start.tv_sec)*1000 + (end.tv_usec-start.tv_usec)/1000;
		printf("%d: %lg\n", num_threads, elapsedTime);
	}
	return 0;
}
