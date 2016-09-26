#include "rdtsc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OFFSET (64*((long long)1)<<20)
#define ATTEMPTS 10000

#define BUFSIZE 1024
#define MAXSIZE 64

char *buffer[BUFSIZE];

__attribute__((optimize("unroll-loops")))
static inline void measure_cache(int size) {
	long long array_size = ((long long)(size+1)) * OFFSET;
	char* array = (char *) malloc((size+1) * OFFSET);
	memset(array, 123, array_size);

	int i, j;
	long long start_time = rdtsc();
	for (i = 0; i < ATTEMPTS; i++){
		for (j = 0; j < size; j++){
			memcpy(buffer, array + j*OFFSET + (i*BUFSIZE)%4096, BUFSIZE);
		}
	}
	long long end_time = rdtsc();
	double time = ((double) (end_time - start_time)) /size / ATTEMPTS;
	printf("%d, %lg\n", size, time);

	free(array);
}

int main() {
	int size;
	for (size = 1; size < MAXSIZE; size++)
		measure_cache(size);
	return 0;
}
