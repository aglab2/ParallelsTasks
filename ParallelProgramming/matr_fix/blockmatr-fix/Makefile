all: test

test: main.o
	c++ -O -o test main.o

main.o: main.cc blockmatr.h
	c++ -O -c main.cc

