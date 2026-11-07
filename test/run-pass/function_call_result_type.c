#include <stdlib.h>

int digit(char* text)
{
	return text[0] - '0';
}

int* identity(int* pointer)
{
	return pointer;
}

int main(void)
{
	int* values = calloc(3, sizeof(int));
	values[1] = 17;
	if(7 * 16 + digit("9") != 121) return 1;
	if(identity(values) + 1 != values + 1) return 2;
	if((identity(values) + 1)[0] != 17) return 3;
	return 0;
}
