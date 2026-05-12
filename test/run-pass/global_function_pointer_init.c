int called;

void set_called()
{
	called = 7;
}

void (*f)() = set_called;

int main()
{
	f();
	return called - 7;
}
