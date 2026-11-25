int add_one(int x)
{
	return x + 1;
}

int add_two(int x)
{
	return x + 2;
}

int main()
{
	int(*outer)(int) = add_one;
	int(*inner)(int) = add_two;
	return outer(inner(0)) - 3;
}
