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

struct token_list* emit(char *s, bool hands_off, struct token_list* head);
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

bool weird(char c)
{
	if(32 > c) return true;
	if(34 == c) return true; // Don't deal with lines with " in them
	if(126 < c) return true;
	return false;
}

/* Parse string to deal with hex characters*/
char table[16] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46};
struct token_list* parse_string(struct token_list* output_list)
{
	char* label;
	static int string_num;
	int i = 1;
	int j = 1;
	int k = 0;
	char* hold = calloc(4096, sizeof(char));
	char* message = calloc(4096, sizeof(char));
	bool hexit = false;

	message[0] = '"';
	while(global_token->s[j] != '"')
	{
		hold[k] = ' ';

		if((global_token->s[j] == 92) & (global_token->s[j + 1] == 'x'))
		{
			hold[k + 1] = upcase(global_token->s[j + 2]);
			hold[k + 2] = upcase(global_token->s[j + 3]);
			int t1 = hex(global_token->s[j + 2], true);
			int t2 = hex(global_token->s[j + 3], false);
			message[i] = t1 + t2;
			if(weird(message[i])) hexit = true;
			j = j + 4;
		}
		else
		{
			hold[k + 1] = table[(global_token->s[j] >> 4)];
			hold[k + 2] = table[(global_token->s[j] & 15)];
			message[i] = global_token->s[j];
			j = j + 1;
		}

		i = i + 1;
		k = k + 3;
	}

	hold[k] = ' ';
	hold[k + 1] = '0';
	hold[k + 2] = '0';
	message[i] = '"';
	message[i + 1] = '\n';

	/* call ... */
	output_list = emit("CALL_IMMEDIATE ", true, output_list);
	asprintf(&label, "%c_STRING_%d\n", 37, string_num);
	output_list = emit(label, true, output_list);

	/* the string */
	if(hexit)
	{
		output_list = emit(hold, false, output_list);
	}
	else
	{
		output_list = emit(message, true, output_list);
	}

	/* The target */
	asprintf(&label, ":_STRING_%d\n", string_num);
	output_list = emit(label, true, output_list);

	/* The cleanup  */
	output_list = emit("POP_eax\n", true, output_list);
	string_num = string_num + 1;
	return output_list;
}
