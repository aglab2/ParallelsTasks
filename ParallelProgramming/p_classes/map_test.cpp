// constructing maps
#include <cstdio>
#include <map>
#include <assert.h>

struct thread_data{
	std::map<int,int> *list;
	int num;
};

#define TEST_SIZE 2048000
#define ASSERT(x, ...) do{ if (!(x)) { printf(__VA_ARGS__); assert(x); } }while(0);

void *thread_func(void *ptr){
	struct thread_data *data = (struct thread_data *) ptr;

	std::map<int,int> *map = data->list;
	int num = data->num;

	int i;

	for (i = 0; i < TEST_SIZE; i++)
		map->insert(std::make_pair<int, int>(num+i, num+i*2));
	for (i = 0; i < TEST_SIZE; i++){
		auto iterator = map->find(num+i);
        ASSERT(iterator != map->end(), "[%d] Fuck my ass at %d", num, num+i);
        int search = iterator->second;
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

int main(int argc, char const *argv[]) {
	clock_t start = clock();
		
	std::map<int,int> *map = new std::map<int,int>();
	struct thread_data data;
	data.list = map;
	data.num = 0;
	
	thread_func(&data);
	free(map);
	
	clock_t end = clock();

	printf("%lg\n", (end-start)/(double)CLOCKS_PER_SEC);	
	return 0;
}
