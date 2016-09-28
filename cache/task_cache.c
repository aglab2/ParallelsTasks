#include "rdtsc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OFFSET (4*((long long)1)<<20)
#define ATTEMPTS 1014
#define MAXOFFSET 512

#define MAXSIZE 64

void cache_clear(){
       FILE *fp = fopen ("/proc/sys/vm/drop_caches", "w");
	if (!fp) return;
       fprintf (fp, "3");
       fclose (fp);
}

static inline void measure_cache(int size) {
	long* array = (long *) malloc((size+1) * OFFSET * sizeof(long));

	int last_array_index = 0;
	for (int offset_num = 0; offset_num < MAXOFFSET / (size+1); offset_num++){
		for (int fragment_num = 0; fragment_num < size; fragment_num++){
			int array_index = fragment_num*OFFSET + offset_num;
			array[array_index] = array_index + OFFSET;
		}
		last_array_index = size*OFFSET + offset_num;
		array[last_array_index] = offset_num + 1;
	}
	array[last_array_index] = 0;

	cache_clear();
	long long start_time = rdtsc();
	for (int i = 0; i < ATTEMPTS; i++){
		int index = array[0];
		while (index != 0) {
			index = array[index];
		}
	}
	long long end_time = rdtsc();
	double time = ((double) (end_time - start_time)) / ATTEMPTS / MAXOFFSET;
	printf("%d, %lg\n", size + 1, time);

	free(array);
}

int main() {
	int size;
	for (size = 0; size < MAXSIZE; size++)
		measure_cache(size);
	return 0;
}
