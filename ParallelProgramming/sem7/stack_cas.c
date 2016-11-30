#include <stdlib.h>
#include <time.h>

#define SLEEP_MAGIC
#ifdef SLEEP_MAGIC
#define FAIL_COUNT 1
#endif

#define CAS __sync_bool_compare_and_swap

struct elem{
	int value;
	struct elem* next;
};

typedef struct elem *stack;

void stack_init(stack* s){
	*s = (struct elem *) malloc(sizeof(struct elem));
	(*s)->value = -666;
}

void stack_fini(stack *s){
	free(*s);
}

void stack_push(stack *s, int value){
	struct elem *push_elem = (struct elem *) malloc(sizeof(struct elem));
	push_elem->value = value;
	int fail = 0;

	do{
		push_elem->next = *s;
#ifdef SLEEP_MAGIC
		if (fail == FAIL_COUNT){
			struct timespec tim, tim2;
			tim.tv_sec = 0;
			tim.tv_nsec = push_elem->value % 10;
			nanosleep(&tim, &tim2);
		}
		fail++;
#endif
	}while(!CAS(s, push_elem->next, push_elem));
}

int stack_pop(stack *s){
	struct elem *pop_elem;
	int fail = 0;
	do{
		pop_elem = *s;
		if (pop_elem->next == NULL) {
			return -1;
		}
#ifdef SLEEP_MAGIC
		if (fail == FAIL_COUNT){
			struct timespec tim, tim2;
			tim.tv_sec = 0;
			tim.tv_nsec = pop_elem->value % 10;
			nanosleep(&tim, &tim2);
		}
		fail++;
#endif
	}while(!CAS(s, pop_elem, pop_elem->next));
	int val = pop_elem->value;
	free(pop_elem);
	return val;
}
