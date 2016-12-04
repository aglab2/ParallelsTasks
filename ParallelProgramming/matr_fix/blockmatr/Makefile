all: test

test: main.o
	c++ -o test main.o

main.o: main.cc blockmatr.h
	c++ -O -c main.cc

