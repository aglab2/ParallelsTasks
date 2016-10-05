#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef unsigned long mytype;

void *func_add(void *arg) {
	volatile mytype *p = (mytype *)arg;
	for (int i = 0; i < 100000000; i++) {
		__sync_fetch_and_add(p, 1);
	}
	return NULL;
}

void *func_sub(void *arg) {
	volatile mytype *p = (mytype *)arg;
	for (int i = 0; i < 100000000; i++) {
		__sync_fetch_and_sub(p, 1);
	}
	return NULL;
}

int main() {
	pthread_t fadd, fsub;
	mytype tmp = 0;
	pthread_create(&fadd, NULL, func_add, &tmp);
	pthread_create(&fsub, NULL, func_sub, &tmp);
	pthread_join(fadd, NULL);
	pthread_join(fsub, NULL);
	printf("tmp=%ld\n", tmp);
}
