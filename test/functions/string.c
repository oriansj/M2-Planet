/* Copyright (C) 2016 Jeremiah Orians
 * This file is part of stage0.
 *
 * stage0 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * stage0 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with stage0.  If not, see <http://www.gnu.org/licenses/>.
 */
#include<stdlib.h>
#define MAX_STRING 4096

char* postpend_char(char* s, char a)
{
	int i = 0;
	while(0 != s[i])
	{
		i = i + 1;
	}
	s[i] = a;
	return s;
}

char* prepend_char(char a, char* s)
{
	int hold = a;
	int prev;
	int i = 0;
	do
	{
		prev = hold;
		hold = s[i];
		s[i] = prev;
		i = i + 1;
	} while(0 != hold);
	return s;
}

char* prepend_string(char* add, char* base)
{
	char* ret = calloc(MAX_STRING, sizeof(char));
	int i = 0;
	while(0 != add[i])
	{
		ret[i] = add[i];
		i = i + 1;
	}

	int j = 0;
	while(0 != base[j])
	{
		ret[i] = base[j];
		i = i + 1;
		j = j + 1;
	}
	return ret;
}
