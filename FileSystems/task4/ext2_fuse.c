#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "ext2_reader.h"

struct ext2_metadata metadata;

static int getattr_callback(const char *path, struct stat *stbuf) {
	memset(stbuf, 0, sizeof(struct stat));

	long inode_number = get_inode_number((char *) path, &metadata);
	if (inode_number > 0){ //This does not work for some reason, idk why
		struct ext2_inode inode = generate_inode(inode_number, &metadata);
		stbuf -> st_mode = inode.i_mode;
		stbuf -> st_ino = inode_number;
		stbuf -> st_mode = inode.i_mode;
		stbuf -> st_uid = inode.i_uid;
		stbuf -> st_gid = inode.i_gid;
		stbuf -> st_size = inode.i_size;
		stbuf -> st_blksize = metadata.block_size;
		stbuf -> st_blocks = inode.i_blocks;
		return 0;
	}else{
		return -ENOENT;
	}
}

static int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	(void) offset;
	(void) fi;

	int inode_number = get_inode_number((char *)path, &metadata);
	if (inode_number < 0)
		return -ENOENT;

	struct ext2_inode inode = generate_inode(inode_number, &metadata);
	struct directory_factory dirf;
	directory_factory_init(&dirf, &inode, &metadata);

	struct ext2_dir_entry_2 *de;

	char filename[EXT2_NAME_LEN];
	while ((de = directory_factory_next(&dirf)) != NULL) {
		struct stat stbuf;
		memset(&stbuf, 0, sizeof(stbuf));
		struct ext2_inode de_inode = generate_inode(de -> inode, &metadata);
		stbuf.st_mode = de_inode.i_mode;
		stbuf.st_ino = de -> inode;
		stbuf.st_mode = de_inode.i_mode;
		stbuf.st_uid = de_inode.i_uid;
		stbuf.st_gid = de_inode.i_gid;
		stbuf.st_size = de_inode.i_size;
		stbuf.st_blksize = metadata.block_size;
		stbuf.st_blocks = de_inode.i_blocks;

		memset(filename, '\0', EXT2_NAME_LEN);
		strncpy(filename, de -> name, de -> name_len);
		if (filler(buf, filename, &stbuf, 0))
			break;
	}

	directory_factory_fini(&dirf);
	return 0;
}


static int open_callback(const char *path, struct fuse_file_info *fi) {
	return 0;
}

static int read_callback(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	int inode_number = get_inode_number((char *)path, &metadata);
	if (inode_number < 0)
		return -ENOENT;

	struct ext2_inode inode = generate_inode(inode_number, &metadata);

	size_t len = inode.i_size;
	if (offset >= (long long) len) {
		return 0;
	}

	struct data_factory df;
	data_factory_init(&df, &inode, &metadata);

	long long left_offset = offset;
	int read_size = 0;
	while (left_offset > 0){
		data_factory_next(&df, &read_size);
		left_offset -= read_size;
	}
	left_offset += read_size; //Recover offset

	if (offset + size > len) {
		//memcpy(buf, filecontent + offset, len - offset);
		data_factory_fini(&df);
		return len - offset;
	}else{
		//memcpy(buf, filecontent + offset, size);
		data_factory_fini(&df);
		return size;
	}
}

static struct fuse_operations fuse_example_operations = {
	.getattr = getattr_callback,
	.open = open_callback,
	.read = read_callback,
	.readdir = readdir_callback,
};

int main(int argc, char *argv[]){
	int fd = open("test", O_RDONLY); //For now it is hardcoded :)

	//Move to superblock
	lseek(fd, 1024, SEEK_SET);
	struct ext2_super_block superblock;
	read(fd, &superblock, sizeof(superblock));

	unsigned block_size = 1 << (superblock.s_log_block_size+10);
	unsigned inode_size = (superblock.s_rev_level == EXT2_GOOD_OLD_REV) ? 128 : superblock.s_inode_size;

	metadata = (struct ext2_metadata) {
		.fd = fd,
		.first_data_block = superblock.s_first_data_block,
		.inodes_per_group = superblock.s_inodes_per_group,
		.group_size = superblock.s_blocks_per_group * block_size,
		.block_size = block_size,
		.inode_size = inode_size,
		.block_size_modifier = ((unsigned) 2 << superblock.s_log_block_size)
	};

	return fuse_main(argc, argv, &fuse_example_operations, NULL);
}
