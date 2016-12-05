all: CFLAGS=-O
all: default 

asan: CFLAGS=-lasan -O -g -fsanitize=address -fno-omit-frame-pointer 
asan: default

debug: CFLAGS=-g 
debug: default

default:
	gcc $(CFLAGS) -c ThreadHelper.c -o thread.o
	gcc $(CFLAGS) -c PLinkedList.c -o list.o
	gcc $(CFLAGS) -c PSkipList.c -o slist.o	
	gcc $(CFLAGS) -lurcu -lpthread PLinkedList_test.c list.o thread.o -o list_test
	gcc $(CFLAGS) -lurcu -lpthread PSkipList_test.c list.o slist.o thread.o -o slist_test
	g++ $(CFLAGS) map_test.cpp -o map_test

clean:
	rm -f list_test slist_test *.o
