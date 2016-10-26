#include "ext2_reader.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void usage_and_die(const char *name) {
	printf("Usage: %s IMAGE [-di DIR_INODE | -d DIR_NAME | -fi FILE_INODE| -f FILE_NAME]\n", name);
	puts("    IMAGE - EXT2 image\n");
	puts("    -di - list directory by inode number\n");
	puts("    -d - list directory by file name\n");
	puts("    -fi - print file by inode number\n");
	puts("    -f - print file by name\n");
	exit(1);
}

int main(int argc, char *argv[]) {
	if (argc < 4) usage_and_die(argv[0]);

	int fd = open(argv[1], O_RDONLY);

	//Move to superblock
	lseek(fd, 1024, SEEK_SET);
	struct ext2_super_block superblock;
	read(fd, &superblock, sizeof(superblock));

	unsigned block_size = 1 << (superblock.s_log_block_size+10);
	unsigned inode_size = (superblock.s_rev_level == EXT2_GOOD_OLD_REV) ? 128 : superblock.s_inode_size;

	struct ext2_metadata metadata = {
		.fd = fd,
		.first_data_block = superblock.s_first_data_block,
		.inodes_per_group = superblock.s_inodes_per_group,
		.group_size = superblock.s_blocks_per_group * block_size,
		.block_size = block_size,
		.inode_size = inode_size,
		.block_size_modifier = ((unsigned) 2 << superblock.s_log_block_size)
	};

	long int inode_number = 0;
	if (strchr(argv[2], 'i')) { //Get inode for user input
		char *end;
		inode_number = strtol(argv[3], &end, 10);

		if (errno != 0 && inode_number == 0) {
    			fprintf(stderr, "Bad inode number!\n");
			close(fd);
    			usage_and_die(argv[0]);
		}
		if (end == argv[2]) {
    			fprintf(stderr, "No digits were found\n");
			close(fd);
    			usage_and_die(argv[0]);
		}
		if (inode_number < 1) {
			fprintf(stderr, "Inode should be >=1\n");
			close(fd);
			usage_and_die(argv[0]);
		}
	}else{ //Generate inode ourselves
		inode_number = get_inode_number(argv[3], &metadata);
	}

	if (strchr(argv[2], 'd')) { //Working with directory listing
		if (traverse_directory(inode_number, NULL, &metadata) == -1){
			fprintf(stderr, "Not a directory!");
		}
		goto _end;
	}

	if (strchr(argv[2], 'f')) { //Working with file
		print_file(inode_number, &metadata);
		goto _end;
	}

_end:	close(fd);
	return 0;
}
