#define _GNU_SOURCE
#include <sched.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/mount.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define STACK_SIZE 1024*1024

static int child_func(void *argv){
	if (mount("none", "/tmp", NULL, MS_PRIVATE|MS_REC, NULL)) {
		printf("Cannot umount tmp: %s\n", strerror(errno));
		exit(1);
	}
	if (mount("tmp", "/tmp", "tmpfs", MS_NOSUID|MS_NOEXEC|MS_NODEV, NULL)) {
		printf("Cannot mount tmpfs: %s\n", strerror(errno));
		exit(1);
	}

	if (mount("none", "/proc", NULL, MS_PRIVATE|MS_REC, NULL)) {
		printf("Cannot umount proc: %s\n", strerror(errno));
		exit(1);
	}
	if (mount("proc", "/proc", "proc", MS_NOSUID|MS_NOEXEC|MS_NODEV, NULL)) {
		printf("Cannot mount proc: %s\n", strerror(errno));
		exit(1);
	}
	printf("Hu!");
	execv("/bin/bash", (char *const *)argv);
	fprintf(stderr, "Failed to start bash: %s\n", strerror(errno));
	exit(2);
}

int main(int argc, char *argv[], char *envp[]){
	char *stack = (char *) malloc(STACK_SIZE);
	if (stack == NULL){
		fprintf(stderr, "Failed to malloc stack: %s\n", strerror(errno));
		exit(1);
	}
	char *stack_top = stack + STACK_SIZE;  /* Assume stack grows downward */

	int pid = clone(child_func, stack_top, SIGCHLD|CLONE_NEWUTS|CLONE_NEWPID|CLONE_NEWIPC|CLONE_NEWNS, argv);
	if (pid == -1) {
		fprintf(stderr, "Failed to clone: %s\n", strerror(errno));
		exit(1);
	}
	int status;
	waitpid(-1, &status, 0);
	return status;

	return 0;
}
