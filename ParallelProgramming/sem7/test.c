#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "stack_cas.c"

#define NUM_THREADS 16

void *worker(void *arg){
	int val = pthread_self() % 100000;

	stack *s = (stack *) arg;

	int i;
	for (i = 0; i < 1000000; i++){
		stack_push(s, val);
		val = stack_pop(s);
	}
	return NULL;
}

int main(void){
	int i = 0;
	int err;

	stack s;
	stack_init(&s);

	pthread_t threads[NUM_THREADS];
	for (i = 0; i < NUM_THREADS; i++){
		err = pthread_create(threads + i, NULL, &worker, &s);
		if (err != 0)
			printf("Failed to create thread: %s", strerror(err));
	}

	for (i = 0; i < NUM_THREADS; i++){
		pthread_join(threads[i], NULL);
	}

	stack_fini(&s);

	return 0;
}
