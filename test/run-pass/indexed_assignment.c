#include <stdlib.h>

struct item
{
	int first;
	int second;
	int third;
	int fourth;
	int fifth;
};

int main(void)
{
	struct item* items = calloc(3, sizeof(struct item));
	struct item** pointers = calloc(2, sizeof(struct item*));
	pointers[0] = items;
	pointers[1] = items + 1;
	if(pointers[1] != items + 1) return 1;
	pointers[0] += 1;
	if(pointers[0] != pointers[1]) return 2;
	char* bytes = calloc(3, sizeof(char));
	bytes[0] = 5;
	bytes[1] = 9;
	bytes[0] += 2;
	if(bytes[0] != 7) return 3;
	if(bytes[1] != 9) return 4;
	return 0;
}
