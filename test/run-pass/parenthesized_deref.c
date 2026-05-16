int main()
{
	int value = 3;
	int* pointer = &value;

	if(*(pointer) != 3) return 1;

	*(pointer) = 4;
	if(value != 4) return 2;

	int** double_pointer = &pointer;
	if(**(double_pointer) != 4) return 3;

	return 0;
}
