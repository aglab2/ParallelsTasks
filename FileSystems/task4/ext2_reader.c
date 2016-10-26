#include <asm/byteorder.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <limits.h>

#define EXT2_S_IFDIR	0x4000	/* directory */
#define PTRSIZE 4

#define INDIRECT_INDEX 12
#define BIDIRECT_INDEX 13
#define TRIDIRECT_INDEX 14

#define EXT2_DIR_ENTRY_HEADER_SIZE (sizeof(struct ext2_dir_entry_2) - EXT2_NAME_LEN - 1)

#define MIN(x,y) (x<y ? x : y)

#include "ext2_reader.h"

/*------------------------------BLOCK_FACTORY---------------------------------*/
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



/*-------------------------------DATA_FACTORY---------------------------------*/
int data_factory_init(struct data_factory *df, struct ext2_inode *inode, struct ext2_metadata *md) {
	block_factory_init(&df->bf, inode, md->block_size, md->fd);
	df -> fd = md -> fd;
	df -> current_data = malloc(md->block_size);
	df -> block_ctr = 0;
	df -> block_cnt = inode->i_blocks / md->block_size_modifier;
	df -> remain_file_size = inode->i_size;
	df -> block_size = md -> block_size;
	return 0;
}

//Returns NULL on no data available
//Don't free acquired data!!!
void *data_factory_next(struct data_factory *df, int *read_size) {
	if (df -> block_ctr >= df -> block_cnt || df -> remain_file_size < 0)
		return NULL;

	struct block_factory *bf = &df -> bf;
	int fd = df -> fd;
	long block_size = df -> block_size;

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




/*----------------------------DIRECTORY_FACTORY-------------------------------*/
int directory_factory_init(struct directory_factory *dirf, struct ext2_inode *inode, struct ext2_metadata *md) {
	data_factory_init(&dirf -> df, inode, md);
	dirf -> current_data = NULL;
	dirf -> cur_read_size = -1;
	dirf -> block_size = md -> block_size;
	return 0;
}

//Returns NULL if no directories left
struct ext2_dir_entry_2 *directory_factory_next(struct directory_factory *dirf) {
	if (dirf -> current_data == NULL) {
		int read_size;
		dirf -> current_data = (char *) data_factory_next(&dirf -> df, &read_size);
		if (!dirf -> current_data) {
			return NULL;
		}
		dirf -> cur_read_size = 0;
	}

	struct ext2_dir_entry_2 *dir_entry = (struct ext2_dir_entry_2 *) (dirf -> current_data + dirf -> cur_read_size);

	if (dir_entry->inode == 0) //End of directory entries -> return NULL
		return NULL;
	dirf -> cur_read_size += dir_entry -> rec_len;
	if (dirf -> cur_read_size >= dirf -> block_size) //Request more data on next call
		dirf -> current_data = NULL;

	return dir_entry;
}

int directory_factory_fini(struct directory_factory *dirf) {
	data_factory_fini(&dirf -> df);
	return 0;
}






/* ----------------------------INODE FUNCTIONS--------------------------------*/
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
//Path needle==NULL to print all directories
int traverse_directory(int inode_number, const char *needle, struct ext2_metadata *md) {
	struct ext2_inode inode = generate_inode(inode_number, md);

	if (!(inode.i_mode & EXT2_S_IFDIR))
		return -1;

	struct directory_factory dirf;
	directory_factory_init(&dirf, &inode, md);

	struct ext2_dir_entry_2 *dir_entry = NULL;
	while((dir_entry = directory_factory_next(&dirf)) != NULL) {
		if (!needle){
			printf("%.*s\n", dir_entry -> name_len, dir_entry -> name);
		}else{
			if (!strncmp(dir_entry -> name, needle, dir_entry -> name_len)) {
				directory_factory_fini(&dirf);
				return dir_entry -> inode;
			}
		}
	}

	directory_factory_fini(&dirf);
	return needle ? -2 : 0;
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

//Returns -1 if no such inode
long get_inode_number(char *path, struct ext2_metadata *md){
	long inode_number = EXT2_ROOT_INO;
	char *base_path, *saveptr, *token;
	for (base_path = path; ;base_path = NULL) {
		token = strtok_r(base_path, "/", &saveptr);
		if (token == NULL)
			break;
		if (!token[0]) //Non empty string
			continue;
		inode_number = traverse_directory(inode_number, token, md);
		if (inode_number < 0)
			return -1;
	}
	return inode_number;
}
