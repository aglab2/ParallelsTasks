#ifndef __EXT2_READER_H
#define __EXT2_READER_H

#include <ext2fs/ext2fs.h>

struct ext2_metadata{
	int fd;
	unsigned first_data_block;
	unsigned inodes_per_group;
	unsigned group_size;
	unsigned block_size;
	unsigned inode_size;
	unsigned block_size_modifier;
};

int traverse_directory(int inode_number, const char *needle, struct ext2_metadata *md);
int print_file(int inode_number, struct ext2_metadata *md);

//Returns -1 if no such inode
long get_inode_number(char *path, struct ext2_metadata *md);
struct ext2_inode generate_inode(int inode_number, struct ext2_metadata *md);




//Factory to generate blocks, made to work with indirect blocks
struct block_factory{
	int fd;
	long int block_size;
	long int cur_index;
	struct ext2_inode *inode;
	unsigned *indirect_source;
};

//Factory to generate data, get from block factory to remove duplication
struct data_factory{
	struct block_factory bf;
	int fd;
	void *current_data;
	long block_ctr;
	long block_cnt;
	long remain_file_size;
	long block_size;
};

//Factory to generate directories, get data from data factory
struct directory_factory{
	struct data_factory df;
	char *current_data;
	long cur_read_size;
	long block_size;
};


int data_factory_init(struct data_factory *df, struct ext2_inode *inode, struct ext2_metadata *md);

//Returns NULL on no data available
//Don't free acquired data!!!
void *data_factory_next(struct data_factory *df, int *read_size);
int data_factory_fini(struct data_factory *df);

int directory_factory_init(struct directory_factory *dirf, struct ext2_inode *inode, struct ext2_metadata *md);
//Returns NULL if no directories left
struct ext2_dir_entry_2 *directory_factory_next(struct directory_factory *dirf);
int directory_factory_fini(struct directory_factory *dirf);

#endif
