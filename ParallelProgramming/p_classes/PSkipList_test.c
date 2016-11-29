#include "PSkipList.h"
#include "ThreadHelper.h"

#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct thread_data{
	struct PSkipList *list;
	int num;
};

#define NUM_THREADS 8
#define TEST_SIZE 100000

#define ASSERT(x, ...) do{ if (!(x)) { printf(__VA_ARGS__); assert(x); } }while(0);

void *thread_func(void *ptr){
	thread_register();

	struct thread_data *data = (struct thread_data *) ptr;

	struct PSkipList *list = data->list;
	int num = data->num;

	int i;

	for (i = 0; i < TEST_SIZE; i++)
		PSkipList_insert(list, num+i, num*100+i*2);
	for (i = 0; i < TEST_SIZE; i++){
		int search = PSkipList_search(list, num+i);
		ASSERT(search == num*100+i*2, "[%d] Fuck my ass at %d: %d!=%d\n", num, num+i, search, num*100+2*i);
	}
	for (i = 0; i < TEST_SIZE; i++)
		PSkipList_delete(list, num+i);
	for (i = 0; i < TEST_SIZE; i++){
		int search = PSkipList_search(list, num+i);
		ASSERT(search == -1, "[%d] Fuck my ass at %d: %d!=-1 for %d\n", num, num+i, search, i);
	}
	return 0;
}

void test_slist(int num_threads){
	struct PSkipList list;
	int i;

	pthread_t *threads = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);
	struct thread_data *data = (struct thread_data *) malloc(sizeof(struct thread_data) * num_threads);

	PSkipList_init(&list);
	for (i = 0; i < num_threads; i++){
		data[i].list = &list;
		data[i].num = TEST_SIZE*i;
		pthread_create(threads+i, NULL, thread_func, data+i);
	}

	for (i = 0; i < num_threads; i++){
		pthread_join(threads[i], NULL);
	}

	thread_reset();

	PSkipList_fini(&list);
}

int main(int argc, char const *argv[]) {
	int num_threads = 0;
	for (num_threads = 1; num_threads < MAX_THREADS; num_threads*=2){
		clock_t start = clock();
		test_slist(num_threads);
		clock_t end = clock();
		printf("%d: %lg\n", num_threads, (end-start)/(double)CLOCKS_PER_SEC);
	}
	return 0;
}
