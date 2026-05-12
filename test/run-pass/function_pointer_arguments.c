int add_one(int x)
{
	return x + 1;
}

int apply(int (*f)(int), int x)
{
	return f(x);
}

int main()
{
	return apply(add_one, 2) - 3;
}
