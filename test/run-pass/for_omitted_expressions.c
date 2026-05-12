int main()
{
	int i = 0;
	int sum = 0;

	for(;;)
	{
		i = i + 1;
		if(i == 3) break;
	}
	if(i != 3) return 1;

	for(; i < 6;)
	{
		sum = sum + i;
		i = i + 1;
	}
	if(sum != 12) return 2;

	for(i = 0; ; i = i + 1)
	{
		if(i == 4) break;
	}
	if(i != 4) return 3;

	return 0;
}
