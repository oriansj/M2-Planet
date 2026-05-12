int main()
{
	int a = 0;
	int b = (a = 3, a + 4);
	if(a != 3) return 1;
	if(b != 7) return 2;

	int c = 0;
	if((c = 5, c == 5) == 0) return 3;

	return 0;
}
