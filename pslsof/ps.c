#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#ifdef DEBUG
#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( 0 )
#else
#define DEBUG_PRINT(...) do{ } while (0)
#endif

int isnumeric (const char *s) {
    if (s == NULL || *s == '\0' || isspace(*s))
      return 0;
    char * p;
    strtol(s, &p, 10);
    return *p == '\0';
}

int main(int argc, char const *argv[]) {
	struct dirent *curproc_dirent;
	DIR *proc_dir;

	if ((proc_dir = opendir("/proc")) == NULL) {
		DEBUG_PRINT("Can't open proc\n");
		return 1;
	}

	char curproc_name[100];
	char curproc_exename[100];
	char curproc_realexename[100];

	while ((curproc_dirent = readdir(proc_dir)) != NULL) {
		struct stat curproc_stat;
		if (!isnumeric(curproc_dirent -> d_name)) {
			continue;
		}
		sprintf(curproc_name, "/proc/%s", curproc_dirent -> d_name);
		if (stat(curproc_name, &curproc_stat) == -1) {
			DEBUG_PRINT("Unable to stat file: %s, error: %s\n", curproc_name, strerror(errno));
			continue;
		}
		if ((curproc_stat.st_mode & S_IFMT) != S_IFDIR) //Not a dir, don't process
			continue;

		sprintf(curproc_exename, "%s/exe", curproc_name);
		int len = 0;
		if ((len = readlink(curproc_exename, curproc_realexename, 99)) == -1) {
			DEBUG_PRINT("Failed to read exe link for %s, error: %s\n", curproc_name, strerror(errno));
			continue;
		}
		curproc_realexename[len] = '\0';
		printf("%s\t%s\n", curproc_dirent -> d_name, curproc_realexename);
	}

	closedir(proc_dir);
	return 0;
}
