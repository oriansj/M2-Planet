int main()
{
	int a = 0;
	int b = 0;

	if(a = 1, a)
	{
		b = b + 1;
	}
	else
	{
		return 1;
	}

	while(a = a + 1, a < 4)
	{
		b = b + 1;
	}
	if(b != 3) return 2;

	do
	{
		b = b - 1;
	}
	while(a = 0, b);
	if(b != 0) return 3;

	for(a = 0, b = 0; a = a + 1, a < 4; b = b + 1)
	{
	}
	if(b != 3) return 4;

	return 0;
}
