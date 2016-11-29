// constructing maps
#include <cstdio>
#include <map>
#include <assert.h>

struct thread_data{
	std::map<int,int> *list;
	int num;
};

#define NUM_THREADS 8
#define TEST_SIZE 1000

#define MAX_THREADS 2

#define ASSERT(x, ...) do{ if (!(x)) { printf(__VA_ARGS__); assert(x); } }while(0);

void *thread_func(void *ptr){
	struct thread_data *data = (struct thread_data *) ptr;

	std::map<int,int> *map = data->list;
	int num = data->num;

	printf("%d\n", num);

	int i;

	for (i = 0; i < TEST_SIZE; i++)
		map->insert(std::pair<int,int>(num+i, num+i*2));
	for (i = 0; i < TEST_SIZE; i++){
		int search = map->find(num+i)->first;
		ASSERT(search == num+i*2, "[%d] Fuck my ass at %d: %d!=%d\n", num, num+i, search, num+2*i);
	}
	for (i = 0; i < TEST_SIZE; i++)
		map->erase(num+i);
	for (i = 0; i < TEST_SIZE; i++){
		auto search = map->find(num+i);
		ASSERT(search == map->end(), "[%d] Fuck my ass at %d: element %d was found!\n", num, num+i, search->first);
	}
	return 0;
}

void test_slist(int num_threads){
	std::map<int,int> *map = new std::map<int,int>();
	int i;

	pthread_t *threads = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);
	struct thread_data *data = (struct thread_data *) malloc(sizeof(struct thread_data) * num_threads);

	for (i = 0; i < num_threads; i++){
		data[i].list = map;
		data[i].num = TEST_SIZE*i;
		pthread_create(threads+i, NULL, thread_func, data+i);
	}

	for (i = 0; i < num_threads; i++){
		pthread_join(threads[i], NULL);
	}

	delete map;
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
