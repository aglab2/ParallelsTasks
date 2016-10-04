#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <assert.h>

unsigned scan_unsigned(){
	unsigned ret = 0;
	assert(sizeof(unsigned) == 4);
	int i;
	for (i = 0; i < 4; i++){
		char c;
		if (scanf("%1s", &c) < 1) {
			errno = ENODATA;
			return ret;
		}
		unsigned unsigned_c = 0xFF & c;
		ret |= (unsigned_c) << (8*(3-i));
	}
	errno = 0;
	return ret;
}

unsigned bits_count(unsigned num){
	unsigned ret = -1;
  	while(num != 0){
		ret++;
		num >>= 1;
	}
	return ret;
}

//Num will be mutated after function execution!
unsigned get_bit(unsigned *num){
	unsigned ret = (*num) >> 31; //Get the biggest bit
	*num <<= 1;
	return ret;
}

//Num will be mutated after function execution!
unsigned generate_byte(unsigned *num, unsigned char init, int bits_to_fil){
	unsigned char byte = init;
	int bit;
	for (bit = 0; bit < bits_to_fil; bit++)
		byte |= get_bit(num) << (bits_to_fil - bit - 1);
	return byte;
}

int main(){
	errno = 0;

	unsigned num = scan_unsigned();
	while (errno == 0){
		unsigned bits = bits_count(num);
		int i;
		//Revied code style
		if (bits <= 7){
			num <<= (32-7); //Byte stream preparaion: reading from big bits
			putchar(generate_byte(&num, 0x00, 7)); //0xxxxxxx
		}else
		if (bits <= 11){
			num <<= (32-11);
			putchar(generate_byte(&num, 0xC0, 5)); //110xxxxx
			for (i = 0; i < 1; i++)
				putchar(generate_byte(&num, 0x80, 6));
		}else
		if (bits <= 16){
			num <<= (32-16);
			putchar(generate_byte(&num, 0xE0, 4)); //1110xxxx
			for (i = 0; i < 2; i++)
				putchar(generate_byte(&num, 0x80, 6));
		}else
		if (bits <= 21){
			num <<= (32-21);
			putchar(generate_byte(&num, 0xF0, 3)); //11110xxx
			for (i = 0; i < 3; i++)
				putchar(generate_byte(&num, 0x80, 6));
		}else
		if (bits <= 26){
			num <<= (32-26);
			putchar(generate_byte(&num, 0xF8, 2)); //111110xx
			for (i = 0; i < 4; i++)
				putchar(generate_byte(&num, 0x80, 6));
		}else
		if (bits <= 31){
			num <<= (32-31);
			putchar(generate_byte(&num, 0xFC, 1)); //1111110x
			for (i = 0; i < 5; i++)
				putchar(generate_byte(&num, 0x80, 6));
		}else{
			fprintf(stderr, "Wrong input!\n");
		}
		num = scan_unsigned();
	}
	if(num != 0){
		fprintf(stderr, "Unexpected EOF!\n");
	}
	return 0;
}
