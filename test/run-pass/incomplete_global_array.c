int forward[];

int read_forward(void)
{
	return forward[1];
}

int forward[] = { 3, 7, 11 };

int main(void)
{
	return read_forward() - 7;
}
