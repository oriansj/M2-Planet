struct Inner
{
	int value;
};

struct Outer
{
	struct Inner inner;
	struct Inner* pointer;
};

int main()
{
	struct Inner inner;
	inner.value = 42;

	struct Outer outer;
	outer.inner.value = 7;
	outer.pointer = &inner;

	int a = (int) outer.inner.value;
	if(a != 7) return 1;

	int b = (int) outer.pointer->value;
	if(b != 42) return 2;

	int c = (int) (&outer)->inner.value;
	if(c != 7) return 3;

	return 0;
}
