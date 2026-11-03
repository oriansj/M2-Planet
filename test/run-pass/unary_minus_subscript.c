/* -array[element] must negate the element, not index a negated pointer. */

int main()
{
	int a[4];
	a[1] = 7;
	if(-a[1] != -7)
	{
		return 1;
	}
	return 0;
}
