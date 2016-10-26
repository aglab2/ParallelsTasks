all:
	gcc -c -O ext2_reader.c
	gcc -o fuse `pkg-config fuse --cflags --libs` ext2_fuse.c ext2_reader.o
	gcc -o reader ext2_reader_test.c ext2_reader.o

clean:
	rm -f *.o fuse reader
