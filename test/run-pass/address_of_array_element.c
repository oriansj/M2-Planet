/* &array[element] must evaluate to the element's address, not its value. */

int get(int* p)
{
	return p[0];
}

int main()
{
	int b[4];
	b[1] = 42;
	if(get(&b[1]) != 42)
	{
		return 1;
	}
	return 0;
}
