#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static __inline__ unsigned long long rdtsc(void) {
	unsigned long long int x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;
}

struct reader_info{
	volatile long long data;
	pthread_spinlock_t spinlock;
	long long ATTEMPTS;
};

void *reader_thread(void *ptr) {
	struct reader_info *ri = (struct reader_info *) ptr;

	long long test = 0;
	int i;
	long long ATTEMPTS = ri -> ATTEMPTS;

	for (i = 0; i < ATTEMPTS; i++){
		pthread_spin_lock(&ri -> spinlock);
		test += ri -> data;
		pthread_spin_unlock(&ri -> spinlock);
	}

	return NULL;
}

int main(){
	struct reader_info ri;

	pthread_spin_init(&ri.spinlock, PTHREAD_PROCESS_SHARED);

	int NUM_THREADS = 0;
	for (NUM_THREADS = 1; NUM_THREADS <= 128; NUM_THREADS*=2){
		//Init threads
		pthread_t *threads = (pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
		pthread_spin_lock(&ri.spinlock);
		int i;
		for (i = 0; i < NUM_THREADS; i++){
			if(pthread_create(threads + i, NULL, reader_thread, &ri)) {
				fprintf(stderr, "Error creating thread\n");
				return 1;
			}
		}
		ri.ATTEMPTS = 400000 * 128 / NUM_THREADS;
		pthread_spin_unlock(&ri.spinlock);

		//Start time measurements -- all threads start at one time
		clock_t start = clock();
		long long ATTEMPTS = ri.ATTEMPTS;
		for (i = 0; i < ATTEMPTS; i++){
			pthread_spin_lock(&ri.spinlock);
			ri.data++;
			pthread_spin_unlock(&ri.spinlock);
		}

		for (i = 0; i < NUM_THREADS; i++){
			if(pthread_join(threads[i], NULL)) {
				fprintf(stderr, "Error joining thread\n");
				return 2;
			}
		}
		//End time measurements
		clock_t end = clock();
		printf("%d, %ld\n", NUM_THREADS, end - start);
		fflush(stdin);
		free(threads);
	}

	pthread_spin_destroy(&ri.spinlock);

	return 0;
}
