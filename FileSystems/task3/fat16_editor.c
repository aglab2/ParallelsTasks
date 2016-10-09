#include <linux/msdos_fs.h>
#include <linux/kernel.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <time.h>

char *get_name(struct msdos_dir_entry dir_entry){
	char *filename = (char *) calloc(13, 1);
	int char_index = 0;
	if (dir_entry.name[0] == 0x05){
		memcpy(filename, dir_entry.name + 1, 7);
		char_index = 6;
	}else{
		memcpy(filename, dir_entry.name, 8);
		char_index = 7;
	}
	while(filename[char_index] == ' '){
		filename[char_index] = '\0';
		char_index--;
	}
	if ((dir_entry.name[8] | dir_entry.name[9] | dir_entry.name[10]) != ' '){
		filename[++char_index] = '.';
		char_index++;
		memcpy(filename + char_index, dir_entry.name + 8, 3);
		char_index += 2;
		while(filename[char_index] == ' ') {
			filename[char_index] = '\0';
			char_index--;
		}
	}
	return filename;
}

struct tm get_creation_time(struct msdos_dir_entry dir_entry){
	struct tm time;
	//hhhhhmmm mmmxxxxx
	time.tm_hour = (dir_entry.ctime & 0xF800) >> 11;
	time.tm_min = (dir_entry.ctime & 0x07E0) >> 5;
	time.tm_sec = (dir_entry.ctime & 0x001F) * 2; //In FAT16 seconds are divided by 2

	//yyyyyyym mmmddddd
	time.tm_year = ((dir_entry.cdate & 0xFE00) >> 9) + 80; //Years in FAT16 are counted from 1980, in tm from 1900
	time.tm_mon = ((dir_entry.cdate & 0x01E0) >> 5) - 1; //Month 8 in FAT16 is 9th month in tm
	time.tm_mday = (dir_entry.cdate & 0x001F);
	return time;
}

struct tm get_access_time(struct msdos_dir_entry dir_entry){
	struct tm time;
	//hhhhhmmm mmmxxxxx
	time.tm_hour = (dir_entry.time & 0xF800) >> 11;
	time.tm_min = (dir_entry.time & 0x07E0) >> 5;
	time.tm_sec = (dir_entry.time & 0x001F) * 2; //In FAT16 seconds are divided by 2

	//yyyyyyym mmmddddd
	time.tm_year = ((dir_entry.adate & 0xFE00) >> 9) + 80; //Years in FAT16 are counted from 1980, in tm from 1900
	time.tm_mon = ((dir_entry.adate & 0x01E0) >> 5) - 1; //Month 8 in FAT16 is 9th month in tm
	time.tm_mday = (dir_entry.adate & 0x001F);
	return time;
}

int print_file(struct msdos_dir_entry dir_entry, short *fat_table, int fd){
	return 0;
}

int main(int argc, char *argv[]){
	if (argc < 2){
		printf("Usage: %s IMAGE [FILE]\n", argv[0]);
		puts("    IMAGE - FAT16 image\n");
		puts("    Not providing argument FILE, program will print out all files\n");
		puts("    File attributes are printed after file\n");
		puts("    R - Read Only, H - hidden file, S - system file\n");
		puts("    Y - Special Entry, D - Directory, M - Modified Flag\n");
		printf("Usage: %s IMAGE [FILE]", argv[0]);
		printf("Usage: %s IMAGE [FILE]", argv[0]);
		exit(1);
	}
	int fd = open(argv[1], O_RDONLY);

	struct fat_boot_sector boot_sector;
	read(fd, &boot_sector, sizeof(boot_sector));

	unsigned short sector_size = __le16_to_cpu(*(__le16 *)boot_sector.sector_size);
	unsigned short dir_entries_count = __le16_to_cpu(*(__le16 *)boot_sector.dir_entries);
	unsigned short cluster_size = boot_sector.sec_per_clus * sector_size;
	unsigned short reserved_size = boot_sector.reserved * sector_size;

	unsigned short FAT_table_size = boot_sector.fat_length * boot_sector.fats * sector_size;
	unsigned short root_directory_size = dir_entries_count * sizeof(struct msdos_dir_entry);

	lseek(fd, reserved_size, SEEK_SET);
	short *fat_table = (short *) malloc(FAT_table_size);
	read(fd, fat_table, FAT_table_size);

	struct msdos_dir_entry *dir_entries = (struct msdos_dir_entry *) malloc(root_directory_size);
	read(fd, dir_entries, root_directory_size);

	int dir_num;
	for (dir_num = 0; dir_num < dir_entries_count; dir_num++){
		struct msdos_dir_entry dir_entry = dir_entries[dir_num];
		if (dir_entry.name[0] == 0x00) continue; //File was never used
		if (dir_entry.name[0] == 0xe5) continue; //File was deleted
		if (dir_entry.attr & 0x40) continue; //Wrong file
		if (dir_entry.attr & 0x80) continue; //Wrong file
		if (dir_entry.attr == 0xF) continue; //TODO: Process files with strange attributes

		char *filename = get_name(dir_entry);
		if (argc < 3){
			printf("%12s | ", filename);
			if (dir_entry.attr & 0x01) putchar('R'); else putchar(' ');
			if (dir_entry.attr & 0x02) putchar('H'); else putchar(' ');
			if (dir_entry.attr & 0x04) putchar('S'); else putchar(' ');
			if (dir_entry.attr & 0x08) putchar('Y'); else putchar(' ');
			if (dir_entry.attr & 0x10) putchar('D'); else putchar(' ');
			if (dir_entry.attr & 0x20) putchar('M'); else putchar(' ');
			printf(" | ");

			struct tm creation_time = get_creation_time(dir_entry);
			printf("%.24s | ", asctime(&creation_time));
			struct tm access_time = get_access_time(dir_entry);
			printf("%.24s\n", asctime(&access_time));
			free(filename);
		}else{
			if (strcmp(filename, argv[2])) {

			}
			free(filename);
			goto END;
		}
	}
END:
	free(fat_table);
	free(dir_entries);

	if (argc < 2)
	return 0;
}
