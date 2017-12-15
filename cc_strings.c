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
int asprintf(char **strp, const char *fmt, ...);

char upcase(char a)
{
	if((97 <= a) && (122 >= a))
	{
		a = a - 32;
	}

	return a;
}

int8_t hex(int c, bool high)
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
		c = c * 16;
	}
	return c;
}

bool weird(char* string)
{
	if(0 == string[0]) return false;
	if('\\' == string[0])
	{
		if('x' == string[1])
		{
			switch(string[2])
			{
				case '0': return true;
				case '1': return true;
				case '8': return true;
				case '9': return true;
				case 'a': return true;
				case 'A': return true;
				case 'b': return true;
				case 'B': return true;
				case 'c': return true;
				case 'C': return true;
				case 'd': return true;
				case 'D': return true;
				case 'e': return true;
				case 'E': return true;
				case 'f': return true;
				case 'F': return true;
				default: return weird(string+3);
			}
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
	message[0] = '"';
	while(string[j] != 0)
	{
		if((string[j] == '\\') & (string[j + 1] == 'x'))
		{
			int t1 = hex(string[j + 2], true);
			int t2 = hex(string[j + 3], false);
			message[i] = t1 + t2;
			j = j + 4;
		}
		else
		{
			message[i] = string[j];
			j = j + 1;
		}

		i = i + 1;
	}

	message[i] = '"';
	message[i + 1] = '\n';
	return message;
}

/* Deal with non-human strings */
char table[16] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46};
char* collect_weird_string(char* string)
{
	int j = 1;
	int k = 1;
	char* hold = calloc(MAX_STRING, sizeof(char));

	hold[0] = '\'';
	while(string[j] != 0)
	{
		hold[k] = ' ';

		if((string[j] == '\\') & (string[j + 1] == 'x'))
		{
			hold[k + 1] = upcase(string[j + 2]);
			hold[k + 2] = upcase(string[j + 3]);
			j = j + 4;
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
	hold[k + 3] = '\'';
	hold[k + 4] = '\n';
	return hold;
}

/* Parse string to deal with hex characters*/
char* parse_string(char* string)
{
	/* the string */
	if(weird(string)) return collect_weird_string(string);
	else return collect_regular_string(string);
}
