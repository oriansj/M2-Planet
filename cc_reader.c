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
FILE* input;
struct token_list* token;

char clearWhiteSpace(char c)
{
	if((32 == c) || (10 == c) || (9 == c)) return clearWhiteSpace(fgetc(input));
	return c;
}

int string_index;
char consume_byte(struct token_list* current, char c)
{
	current->s[string_index] = c;
	string_index = string_index + 1;
	return fgetc(input);
}

char consume_word(struct token_list* current, char c, char frequent)
{
	c = consume_byte(current, c);
	while(c != frequent) c = consume_byte(current, c);
	return fgetc(input);
}

char preserve_keyword(struct token_list* current, char c)
{
	while((('a' <= c) & (c <= 'z')) | (('A' <= c) & (c <= 'Z')) | (('0' <= c) & (c <= '9')) | (c == '_'))
	{
		c = consume_byte(current, c);
	}
	return c;
}

char preserve_symbol(struct token_list* current, char c)
{
	while((c == '<') | (c == '=') | (c == '>') | (c == '|') | (c == '&') | (c == '!') | (c == '-'))
	{
		c = consume_byte(current, c);
	}
	return c;
}

char purge_macro(int ch)
{
	while(10 != ch) ch = fgetc(input);
	return ch;
}

int get_token(int c)
{
	struct token_list* current = calloc(1, sizeof(struct token_list));
	current->s = calloc(MAX_STRING, sizeof(char));

reset:
	string_index = 0;

	c = clearWhiteSpace(c);
	if('#' == c)
	{
		c = purge_macro(c);
		goto reset;
	}
	else if((('a' <= c) & (c <= 'z')) | (('A' <= c) & (c <= 'Z')) | (('0' <= c) & (c <= '9')) | (c == '_'))
	{
		c = preserve_keyword(current, c);
	}
	else if((c == '<') | (c == '=') | (c == '>') | (c == '|') | (c == '&') | (c == '!') | ( c == '-'))
	{
		c = preserve_symbol(current, c);
	}
	else if(c == '\'')
	{
		c = consume_word(current, c, '\'');
	}
	else if(c == '"')
	{
		c = consume_word(current, c, '"');
	}
	else if(c == '/')
	{
		c = consume_byte(current, c);
		if(c == '*')
		{
			c = fgetc(input);
			while(c != '/')
			{
				while(c != '*') c = fgetc(input);
				c = fgetc(input);
			}
			c = fgetc(input);
			goto reset;
		}
		else if(c == '/')
		{
			c = fgetc(input);
			goto reset;
		}
	}
	else if(c != EOF) c = consume_byte(current, c);

	current->prev = token;
	current->next = token;
	token = current;
	return c;
}

struct token_list* reverse_list(struct token_list* head)
{
	struct token_list* root = NULL;
	while(NULL != head)
	{
		struct token_list* next = head->next;
		head->next = root;
		root = head;
		head = next;
	}
	return root;
}

struct token_list* read_all_tokens(char* source_file)
{
	input  = fopen(source_file, "r");
	int ch =fgetc(input);
	while(EOF != ch) ch = get_token(ch);

	return reverse_list(token);
}
