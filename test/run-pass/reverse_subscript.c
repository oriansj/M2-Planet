int global[] = {10, 20, 30};
char text[] = {1, 2, 3};

int main()
{
	int* p = global;
	if(1[p] != 20) return 1;
	if(2[p] != 30) return 2;

	0[p] = 11;
	if(global[0] != 11) return 3;

	char* c = text;
	if(1[c] != 2) return 4;
	2[c] = 4;
	if(2[c] != 4) return 5;

	return 0;
}
