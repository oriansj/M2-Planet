int main()
{
	int a = 0;
	int b = 0;

	a = 1, b = a + 2;
	if(b != 3) return 1;

	if((1 ? a = 3, a + 4 : 0) != 7) return 2;
	if((0 ? 1 : a = 5, a + 6) != 11) return 3;

	return a = 8, a - 8;
}
