#include "ThreadHelper.h"

__thread int thread_num = 0;
int thread_count = 1;

int thread_register(){
	thread_num = __atomic_fetch_add(&thread_count, 1, __ATOMIC_RELAXED);
	srand48_r(time(NULL), &data[thread_num]);
	return thread_num;
}

void thread_reset(){
	thread_count = 1;
}

//TODO: This is bullshit
int thread_randbit(){
	double result;
	drand48_r(&data[thread_num], &result);
	return result < 0.5;
}

void *thread_markPointer(void *ptr){
	return (void *)((long)ptr | 1);
}

void *thread_unmarkPointer(void *ptr){
	return (void *)((long)ptr & (~(long)0 - 1));
}

int thread_isMarked(void *ptr){
	return (long)ptr & 1;
}
