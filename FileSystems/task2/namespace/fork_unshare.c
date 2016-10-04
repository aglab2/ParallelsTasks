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

int main(int argc, char *argv[], char *envp[]){
	//This is a little bit too excessive, but it works :)
	int unshare_flags = CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWIPC | CLONE_NEWCGROUP;
	if (unshare(unshare_flags) == -1){
		fprintf(stderr, "Failed to unshare: %s\n", strerror(errno));
		exit(1);
	}
	int pid = fork();
	if (pid == -1) {
		fprintf(stderr, "Failed to clone: %s\n", strerror(errno));
		exit(1);
	}

	if (pid != 0) {
		int status;
		waitpid(-1, &status, 0);
		return status;
	}

	if (mount("none", "/tmp", NULL, MS_PRIVATE|MS_REC, NULL)) {
		printf("Cannot umount tmp: %s\n", strerror(errno));
		exit(1);
	}
	if (mount("proc", "/tmp", "tmpfs", MS_NOSUID|MS_NOEXEC|MS_NODEV, NULL)) {
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


	execve("/bin/bash", argv, envp);
	fprintf(stderr, "Failed to start bash: %s\n", strerror(errno));
	exit(2);

	return 0;
}
