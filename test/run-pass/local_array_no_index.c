/* Regression test, verifies a local array without an index yields a
 * pointer to itself.
 */

int first(int* p)
{
	return p[0];
}

int main()
{
	int b[2];
	b[0] = 30;
	b[1] = 12;

	if(first(b) != 30)
	{
		return 1;
	}
	if(*b != 30)
	{
		return 2;
	}
	*b = 42;
	if(b[0] != 42)
	{
		return 3;
	}
	return 0;
}
