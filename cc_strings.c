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

#include "cc.h"
#include <stdint.h>

struct token_list* emit(char *s, struct token_list* head);

char upcase(char a)
{
	if((97 <= a) && (122 >= a))
	{
		a = a - 32;
	}

	return a;
}

int hex(int c, int high)
{
	if (c >= '0' && c <= '9')
	{
		c = (c - 48);
	}
	else if (c >= 'a' && c <= 'z')
	{
		c = (c - 87);
	}
	else if (c >= 'A' && c <= 'Z')
	{
		c = (c - 55);
	}
	else
	{
		exit(EXIT_FAILURE);
	}

	if(high)
	{
		c = c << 4;
	}
	return c;
}

int weird(char* string)
{
	if(0 == string[0]) return FALSE;
	if('\\' == string[0])
	{
		if('x' == string[1])
		{
			if('0' == string[2]) return TRUE;
			else if('1' == string[2]) return TRUE;
			else if('8' == string[2]) return TRUE;
			else if('9' == string[2]) return TRUE;
			else if('a' == string[2]) return TRUE;
			else if('A' == string[2]) return TRUE;
			else if('b' == string[2]) return TRUE;
			else if('B' == string[2]) return TRUE;
			else if('c' == string[2]) return TRUE;
			else if('C' == string[2]) return TRUE;
			else if('d' == string[2]) return TRUE;
			else if('D' == string[2]) return TRUE;
			else if('e' == string[2]) return TRUE;
			else if('E' == string[2]) return TRUE;
			else if('f' == string[2]) return TRUE;
			else if('F' == string[2]) return TRUE;
			else return weird(string+3);
		}
		else if('n' == string[1])
		{
			return weird(string+2);
		}
		else if('t' == string[1])
		{
			return weird(string+2);
		}
		else
		{
			return weird(string+3);
		}
	}
	return weird(string+1);
}

/* Deal with human strings */
char* collect_regular_string(char* string)
{
	int j = 0;
	int i = 0;
	char* message = calloc(MAX_STRING, sizeof(char));

	/* 34 == " */
	message[0] = 34;
	while(string[j] != 0)
	{
		if((string[j] == '\\') & (string[j + 1] == 'x'))
		{
			int t1 = hex(string[j + 2], TRUE);
			int t2 = hex(string[j + 3], FALSE);
			message[i] = t1 + t2;
			j = j + 4;
		}
		else if((string[j] == '\\') & (string[j + 1] == 'n'))
		{
			message[i] = 10;
			j = j + 2;
		}
		else if((string[j] == '\\') & (string[j + 1] == 't'))
		{
			message[i] = 9;
			j = j + 2;
		}
		else
		{
			message[i] = string[j];
			j = j + 1;
		}

		i = i + 1;
	}

	message[i] = 34;
	message[i + 1] = LF;
	return message;
}

/* Deal with non-human strings */
char* collect_weird_string(char* string)
{
	int j = 1;
	int k = 1;
	char* table = "0123456789ABCDEF";
	char* hold = calloc(MAX_STRING, sizeof(char));

	/* 39 == ' */
	hold[0] = 39;
	while(string[j] != 0)
	{
		hold[k] = ' ';

		if((string[j] == '\\') & (string[j + 1] == 'x'))
		{
			hold[k + 1] = upcase(string[j + 2]);
			hold[k + 2] = upcase(string[j + 3]);
			j = j + 4;
		}
		else if((string[j] == '\\') & (string[j + 1] == 'n'))
		{
			hold[k + 1] = '0';
			hold[k + 2] = 'A';
			j = j + 2;
		}
		else if((string[j] == '\\') & (string[j + 1] == 't'))
		{
			hold[k + 1] = '0';
			hold[k + 2] = '9';
			j = j + 2;
		}
		else
		{
			hold[k + 1] = table[(string[j] >> 4)];
			hold[k + 2] = table[(string[j] & 15)];
			j = j + 1;
		}

		k = k + 3;
	}

	hold[k] = ' ';
	hold[k + 1] = '0';
	hold[k + 2] = '0';
	hold[k + 3] = 39;
	hold[k + 4] = LF;
	return hold;
}

/* Parse string to deal with hex characters*/
char* parse_string(char* string)
{
	/* the string */
	if(weird(string)) return collect_weird_string(string);
	else return collect_regular_string(string);
}
