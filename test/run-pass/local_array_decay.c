int main()
{
	char s[3];
	char* p = s;
	int a[3];
	int* q = a;

	s[0] = 1;
	s[1] = 2;
	s[2] = 3;

	if(*p != 1) return 1;
	if(p[1] != 2) return 2;
	if(p[2] != 3) return 3;

	a[0] = 10;
	a[1] = 20;
	a[2] = 30;

	if(*q != 10) return 4;
	if(q[1] != 20) return 5;
	if(q[2] != 30) return 6;

	return 0;
}
