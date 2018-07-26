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
int char2hex(int c);

char upcase(char a)
{
	if((97 <= a) && (122 >= a))
	{
		a = a - 32;
	}

	return a;
}

int hexify(int c, int high)
{
	int i = char2hex(c);

	if(0 > i)
	{
		file_print("Tried to print non-hex number\n", stderr);
		exit(EXIT_FAILURE);
	}

	if(high)
	{
		i = i << 4;
	}
	return i;
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
			else if('2' == string[2])
			{
				if('2' == string[3]) return TRUE;
				else return weird(string+3);
			}
			else if('3' == string[2])
			{
				if('A' == string[3]) return TRUE;
				else return weird(string+3);
			}
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
			if(':' == string[2]) return TRUE;
			return weird(string+2);
		}
		else if('t' == string[1])
		{
			return weird(string+2);
		}
		else if('"' == string[1]) return TRUE;
		else
		{
			return weird(string+3);
		}
	}
	return weird(string+1);
}

/* Lookup escape values */
int escape_lookup(char* c)
{
	if((c[0] == '\\') & (c[1] == 'x'))
	{
		int t1 = hexify(c[2], TRUE);
		int t2 = hexify(c[3], FALSE);
		return t1 + t2;
	}
	else if((c[0] == '\\') & (c[1] == 'n')) return 10;
	else if((c[0] == '\\') & (c[1] == 't')) return 9;
	else if((c[0] == '\\') & (c[1] == '\\')) return 92;
	else if((c[0] == '\\') & (c[1] == '\'')) return 39;
	else if((c[0] == '\\') & (c[1] == '"')) return 34;
	else if((c[0] == '\\') & (c[1] == 'r')) return 13;

	file_print("Unknown escape recieved: ", stderr);
	file_print(c, stderr);
	file_print(" Unable to process\n", stderr);
	exit(EXIT_FAILURE);
}

/* Deal with human strings */
char* collect_regular_string(char* string)
{
	int j = 0;
	string_index = 0;

	/* 34 == " */
	hold_string[0] = 34;
	while(string[j] != 0)
	{
		if((string[j] == '\\') & (string[j + 1] == 'x'))
		{
			hold_string[string_index] = escape_lookup(string + j);
			j = j + 4;
		}
		else if(string[j] == '\\')
		{
			hold_string[string_index] = escape_lookup(string + j);
			j = j + 2;
		}
		else
		{
			hold_string[string_index] = string[j];
			j = j + 1;
		}

		string_index = string_index + 1;
	}

	char* message = calloc(string_index + 3, sizeof(char));
	copy_string(message, hold_string);
	reset_hold_string();
	message[string_index] = 34;
	message[string_index + 1] = LF;
	return message;
}

/* Deal with non-human strings */
char* collect_weird_string(char* string)
{
	int j = 1;
	string_index = 1;
	int temp;
	char* table = "0123456789ABCDEF";

	/* 39 == ' */
	hold_string[0] = 39;
	while(string[j] != 0)
	{
		hold_string[string_index] = ' ';

		if((string[j] == '\\') & (string[j + 1] == 'x'))
		{
			hold_string[string_index + 1] = upcase(string[j + 2]);
			hold_string[string_index + 2] = upcase(string[j + 3]);
			j = j + 4;
		}
		else if(string[j] == '\\')
		{
			temp = escape_lookup(string + j);
			hold_string[string_index + 1] = table[(temp >> 4)];
			hold_string[string_index + 2] = table[(temp & 15)];
			j = j + 2;
		}
		else
		{
			hold_string[string_index + 1] = table[(string[j] >> 4)];
			hold_string[string_index + 2] = table[(string[j] & 15)];
			j = j + 1;
		}

		string_index = string_index + 3;
	}

	char* hold = calloc(string_index + 6, sizeof(char));
	copy_string(hold, hold_string);
	reset_hold_string();
	hold[string_index] = ' ';
	hold[string_index + 1] = '0';
	hold[string_index + 2] = '0';
	hold[string_index + 3] = 39;
	hold[string_index + 4] = LF;
	return hold;
}

/* Parse string to deal with hex characters*/
char* parse_string(char* string)
{
	/* the string */
	if((weird(string)) || ':' == string[1]) return collect_weird_string(string);
	else return collect_regular_string(string);
}
