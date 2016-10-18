#include <ext2fs/ext2fs.h>
#include <asm/byteorder.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define	EXT2_S_IFMT	0xF000	/* format mask  */
#define	EXT2_S_IFSOCK	0xC000	/* socket */
#define	EXT2_S_IFLNK	0xA000	/* symbolic link */
#define	EXT2_S_IFREG	0x8000	/* regular file */
#define	EXT2_S_IFBLK	0x6000	/* block device */
#define	EXT2_S_IFDIR	0x4000	/* directory */
#define	EXT2_S_IFCHR	0x2000	/* character device */
#define	EXT2_S_IFIFO	0x1000	/* fifo */

#define PTRSIZE 4

#define INDIRECT_INDEX 12
#define BIDIRECT_INDEX 13
#define TRIDIRECT_INDEX 14

#define EXT2_DIR_ENTRY_HEADER_SIZE (sizeof(struct ext2_dir_entry_2) - EXT2_NAME_LEN - 1)

#define MIN(x,y) (x<y ? x : y)

struct ext2_metadata{
	int fd;
	unsigned first_data_block;
	unsigned inodes_per_group;
	unsigned group_size;
	unsigned block_size;
	unsigned inode_size;
	unsigned block_size_modifier;
};

void usage_and_die(const char *name) {
	printf("Usage: %s IMAGE [-di DIR_INODE | -d DIR_NAME | -fi FILE_INODE| -f FILE_NAME]\n", name);
	puts("    IMAGE - EXT2 image\n");
	puts("    -di - list directory by inode number\n");
	puts("    -d - list directory by file name\n");
	puts("    -fi - print file by inode number\n");
	puts("    -f - print file by name\n");
	exit(1);
}

struct block_factory{
	int fd;
	long int block_size;
	long int cur_index;
	struct ext2_inode *inode;
	unsigned *indirect_source;
};

struct data_factory{
	struct block_factory bf;
	void *current_data;
	long block_ctr;
	long block_cnt;
	long remain_file_size;
};

int block_factory_init(struct block_factory *bf, struct ext2_inode *inode, long int block_size, int fd) {
	bf -> fd = dup(fd);
	bf -> block_size = block_size;
	bf -> cur_index = 0;
	bf -> inode = inode;
	bf -> indirect_source = NULL;
	return 0;
}

//Return NULL if no more blocks available
unsigned int block_factory_next(struct block_factory *bf) {
	if (bf -> cur_index < INDIRECT_INDEX){ //Direct blocks
		long int cur_index = bf -> cur_index++;
		return bf -> inode -> i_block[cur_index];
	}
	if (bf -> cur_index == INDIRECT_INDEX){
		long int indirect_ptr = (bf->inode->i_block[INDIRECT_INDEX]) * bf->block_size;
		lseek(bf -> fd, indirect_ptr, SEEK_SET);
		bf -> indirect_source = (unsigned *) malloc(bf -> block_size);
		read(bf -> fd, bf -> indirect_source, bf -> block_size);
	}

	if (bf -> cur_index < INDIRECT_INDEX + bf -> block_size / PTRSIZE){ //Indirect blocks
		long int cur_index = bf -> cur_index++;
		return bf->indirect_source[cur_index - INDIRECT_INDEX];
	}
	errno = ENOTSUP;
	return 0; //TODO: Other types of indirection (I am lazy)
}

int block_factory_fini(struct block_factory *bf) {
	close(bf -> fd);
	if (bf -> indirect_source) free(bf -> indirect_source);
	return 0;
}

int data_factory_init(struct data_factory *df, struct ext2_inode *inode, struct ext2_metadata *md) {
	block_factory_init(&df->bf, inode, md->block_size, md->fd);
	df -> current_data = malloc(md->block_size);
	df -> block_ctr = 0;
	df -> block_cnt = inode->i_blocks / md->block_size_modifier;
	df -> remain_file_size = inode->i_size;
	return 0;
}

//Returns NULL on no data available
void *data_factory_next(struct data_factory *df, int *read_size) {
	if (df -> block_ctr >= df -> block_cnt || df -> remain_file_size < 0)
		return NULL;

	struct block_factory *bf = &df -> bf;
	int fd = bf -> fd;
	long block_size = bf -> block_size;

	unsigned int cur_block = block_factory_next(bf);

	lseek(fd, cur_block * block_size, SEEK_SET);
	*read_size = MIN(df -> remain_file_size, (int) block_size);
	read(fd, df -> current_data, *read_size);

	df -> block_ctr++;
	df -> remain_file_size -= block_size;

	return df -> current_data;
}

int data_factory_fini(struct data_factory *df) {
	block_factory_fini(&df->bf);
	free(df -> current_data);
	return 0;
}

struct ext2_inode generate_inode(int inode_number, struct ext2_metadata *md){
	int fd = md -> fd;
	int first_data_block = md -> first_data_block;
	unsigned inodes_per_group = md -> inodes_per_group;
	unsigned group_size = md -> group_size;
	unsigned block_size = md -> block_size;
	unsigned inode_size = md -> inode_size;

	long int inode_block_group = (inode_number - 1) / inodes_per_group;
	long int inode_index = (inode_number - 1) % inodes_per_group;

	long int block_group_offset = inode_block_group * group_size;

	//Block group description table -- located in the next block after first_data_block
	struct ext2_group_desc groupdesc;
	lseek(fd, block_group_offset + (first_data_block+1)*block_size, SEEK_SET);
	read(fd, &groupdesc, sizeof(groupdesc));

	long int inode_table = block_group_offset + groupdesc.bg_inode_table*block_size;

	struct ext2_inode inode;
	lseek(fd, inode_table + inode_index*inode_size, SEEK_SET);
	read(fd, &inode, sizeof(inode));

	return inode;
}

//Returns -1 if file is not a directory
int list_directory(int inode_number, struct ext2_metadata *md) {
	int fd = md -> fd;
	unsigned block_size = md -> block_size;
	unsigned block_size_modifier = md -> block_size_modifier;

	struct ext2_inode inode = generate_inode(inode_number, md);

	struct block_factory bf;
	block_factory_init(&bf, &inode, block_size, fd);

	if (!(inode.i_mode & EXT2_S_IFDIR))
		return -1;

	int block_ctr = 0;
	int block_cnt = inode.i_blocks / block_size_modifier;
	for (block_ctr = 0; block_ctr < block_cnt; block_ctr++){
		int cur_block = block_factory_next(&bf);

		struct ext2_dir_entry_2 dir_entry;
		long int cur_read_size = 0;

		while(cur_read_size < block_size) { //Dirent structures should be aligned to block_size
			lseek(fd, cur_block*block_size + cur_read_size, SEEK_SET);
			read(fd, &dir_entry, EXT2_DIR_ENTRY_HEADER_SIZE);

			if (dir_entry.inode == 0) //End of directory entries -> quiting
				break;

			char *file_name = (char *) malloc(dir_entry.name_len + 1);
			file_name[dir_entry.name_len] = '\0';
			read(fd, file_name, dir_entry.name_len);
			printf("%s\n", file_name);

			free(file_name);

			cur_read_size += dir_entry.rec_len;
		}
	}

	block_factory_fini(&bf);
	return 0;
}

int print_file(int inode_number, struct ext2_metadata *md) {
	struct ext2_inode inode = generate_inode(inode_number, md);

	struct data_factory df;
	data_factory_init(&df, &inode, md);

	int read_size;
	void *data = NULL;
	while((data = data_factory_next(&df, &read_size)) != NULL) {
		write(STDOUT_FILENO, data, read_size);

	}

	data_factory_fini(&df);
	return 0;
}

int main(int argc, char const *argv[]) {
	if (argc < 3) usage_and_die(argv[0]);

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
	}



	if (strchr(argv[2], 'd')) { //Working with directory listing
		if (list_directory(inode_number, &metadata) == -1){
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
