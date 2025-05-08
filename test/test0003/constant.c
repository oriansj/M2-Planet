/* Copyright (C) 2016 Jeremiah Orians
 * This file is part of M2-Planet.
 *
 * M2-Planet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * M2-Planet is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with M2-Planet.  If not, see <http://www.gnu.org/licenses/>.
 */
#include<stdlib.h>
#include<stdio.h>

enum
{
	FALSE = 0,
	TRUE = 1,
	H = 72,
	e = 101,
	l = 108,
	o = 111,
	space = 32,
	newline = 10,
	m = 109,
	s = 115,
};

int main()
{
	if(TRUE)
	{
		putchar(H);
		putchar(e);
		putchar(l);
		putchar(l);
		putchar(o);
		putchar(space);
	}
	else
	{
		exit(2);
	}

	if(FALSE)
	{
		exit(3);
	}
	else
	{
		putchar(m);
		putchar(e);
		putchar(s);
		putchar(newline);
	}

	if(1 < 2)
	{
		exit(42);
	}

	return 1;
}
