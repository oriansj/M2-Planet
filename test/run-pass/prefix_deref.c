char s[3];

int main()
{
	char* p = s;
	s[0] = 1;
	s[1] = 2;
	s[2] = 3;

	if(*++p != 2) return 1;
	if(*--p != 1) return 2;

	return 0;
}
