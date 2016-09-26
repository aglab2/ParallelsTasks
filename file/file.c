#include <unistd.h>
#include <fcntl.h>

int main(int argc, char const *argv[]) {
	int fd = open("/tmp", O_RDWR | __O_TMPFILE);
	close(fd);
	return 0;
}
