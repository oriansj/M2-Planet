#include <stdint.h>

int main(void)
{
	int16_t negative = -3;
	uint16_t positive = 7;
	uint16_t zero = 0;
	if((1 ? negative : positive) >= zero) return 1;
	if((0 ? positive : negative) >= zero) return 2;

	/* M2's native integer types all have register width. */
	long long signed_word = -1;
	unsigned long unsigned_word = 0;
	if((1 ? signed_word : unsigned_word) < 0) return 3;
	if((0 ? unsigned_word : signed_word) < 0) return 4;
	unsigned_word = 0 - 1;
	unsigned long long higher_rank = 0;
	long long signed_zero = 0;
	if((1 ? unsigned_word : higher_rank) < signed_zero) return 5;

	char bytes[2];
	bytes[1] = 73;
	char* p = bytes;
	if((1 ? p : 0)[1] != 73) return 6;
	if((0 ? 0 : p)[1] != 73) return 7;
	return 0;
}
