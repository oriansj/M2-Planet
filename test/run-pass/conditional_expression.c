int choose(int flag)
{
	return flag ? 11 : 22;
}

int main()
{
	int a = 1 ? 2 : 3;
	if(a != 2) return 1;

	int b = 0 ? 2 : 3;
	if(b != 3) return 2;

	int c = choose(1) == 11 ? choose(0) : 4;
	if(c != 22) return 3;

	int d = 0 ? 1 : 1 ? 5 : 6;
	if(d != 5) return 4;

	return 0;
}
