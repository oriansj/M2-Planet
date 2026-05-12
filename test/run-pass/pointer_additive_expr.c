int main()
{
	char c[3];
	char* cp = c;
	int i[3];
	int* ip = i;

	c[0] = 1;
	c[1] = 2;
	c[2] = 3;

	if(*(cp + 1) != 2) return 1;
	if(*(1 + cp) != 2) return 2;
	if(*(c + 2) != 3) return 3;

	i[0] = 10;
	i[1] = 20;
	i[2] = 30;

	if(*(ip + 1) != 20) return 4;
	if(*(1 + ip) != 20) return 5;
	if(*(i + 2) != 30) return 6;
	if(*(ip + (2 - 1)) != 20) return 7;
	if(*(ip - -1) != 20) return 8;

	return 0;
}
