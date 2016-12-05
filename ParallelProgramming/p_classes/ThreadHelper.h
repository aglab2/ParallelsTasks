#ifndef __THREAD_HELPER_H
#define __THREAD_HELPER_H

#include <stdlib.h>
#include <time.h>

#define MAX_THREADS 128
struct drand48_data data[MAX_THREADS];

extern __thread int thread_num;
extern int thread_count;

int thread_register();
void thread_reset();
int thread_randbit();

void *thread_markPointer(void *ptr);
void *thread_unmarkPointer(void *ptr);
int thread_isMarked(void *ptr);

#endif
