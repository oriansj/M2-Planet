#include <stdint.h>
#include <stdlib.h>

int main(void)
{
	/* Nonzero adjacent elements expose over-wide loads. */
	char* bytes = calloc(4, 1);
	bytes[1] = 73;
	bytes[2] = 29;
	if(bytes[1] != 73) return 1;
	signed char* signed_bytes = (signed char*)bytes;
	signed_bytes[1] = -7;
	if(signed_bytes[1] != -7) return 2;

	uint16_t* words = calloc(4, sizeof(uint16_t));
	words[1] = 60001;
	words[2] = 999;
	if(words[1] != 60001) return 3;
	int16_t* signed_words = (int16_t*)words;
	signed_words[1] = -300;
	if(signed_words[1] != -300) return 4;
	return 0;
}
