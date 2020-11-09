/* -*- c-file-style: "linux";indent-tabs-mode:t -*- */
/* Copyright (C) 2016 Jeremiah Orians
 * Copyright (C) 2017 Jan Nieuwenhuizen <janneke@gnu.org>
 * This file is part of mescc-tools.
 *
 * mescc-tools is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mescc-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mescc-tools.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

//CONSTANT max_string 4096
#define max_string 4096
//CONSTANT MACRO 1
#define MACRO 1
//CONSTANT STR 2
#define STR 2
//CONSTANT NEWLINE 3
#define NEWLINE 3

//CONSTANT TRUE 1
#define TRUE 1
//CONSTANT FALSE 0
#define FALSE 0

void file_print(char* s, FILE* f);
int match(char* a, char* b);
int string_length(char* a);
char* numerate_number(int a);
int numerate_string(char *a);
int hex2char(int c);
int in_set(int c, char* s);

FILE* source_file;
FILE* destination_file;
int BigEndian;
int BigBitEndian;
int ByteMode;
int Architecture;
int linenumber;

void line_error(char* filename, int linenumber)
{
	file_print(filename, stderr);
	file_print(":", stderr);
	file_print(numerate_number(linenumber), stderr);
	file_print(" :", stderr);
}

struct Token
{
	struct Token* next;
	int type;
	char* Text;
	char* Expression;
	char* filename;
	int linenumber;
};

struct Token* newToken(char* filename, int linenumber)
{
	struct Token* p;

	p = calloc (1, sizeof (struct Token));
	if (NULL == p)
	{
		file_print("calloc failed.\n", stderr);
		exit (EXIT_FAILURE);
	}

	p->filename = filename;
	p->linenumber = linenumber;

	return p;
}

struct Token* reverse_list(struct Token* head)
{
	struct Token* root = NULL;
	struct Token* next;
	while(NULL != head)
	{
		next = head->next;
		head->next = root;
		root = head;
		head = next;
	}
	return root;
}

void purge_lineComment()
{
	int c = fgetc(source_file);
	while(!in_set(c, "\n\r"))
	{
		c = fgetc(source_file);
	}
}

struct Token* append_newline(struct Token* head, char* filename)
{
	linenumber = linenumber + 1;
	if(NULL == head) return NULL;
	if(NEWLINE == head->type)
	{/* Don't waste whitespace*/
		return head;
	}

	struct Token* lf = newToken(filename, linenumber);
	lf->type = NEWLINE;
	lf->next = head;
	lf->Text = "\n";
	lf->Expression = lf->Text;
	return lf;
}


struct Token* store_atom(struct Token* head, char c, char* filename)
{
	char* store = calloc(max_string + 1, sizeof(char));
	if(NULL == store)
	{
		file_print("Exhusted available memory\n", stderr);
		exit(EXIT_FAILURE);
	}
	int ch = c;
	int i = 0;
	do
	{
		store[i] = ch;
		ch = fgetc(source_file);
		i = i + 1;
	} while (!in_set(ch, "\t\n ") && (i <= max_string));

	head->Text = store;
	if('\n' == ch)
	{
		return append_newline(head, filename);
	}
	return head;
}

char* store_string(char c, char* filename)
{
	char* store = calloc(max_string + 1, sizeof(char));
	if(NULL == store)
	{
		file_print("Exhusted available memory\n", stderr);
		exit(EXIT_FAILURE);
	}
	int ch = c;
	int i = 0;
	do
	{
		store[i] = ch;
		i = i + 1;
		ch = fgetc(source_file);
		if(-1 == ch)
		{
			line_error(filename, linenumber);
			file_print("Unmatched \"!\n", stderr);
			exit(EXIT_FAILURE);
		}
		if(max_string == i)
		{
			line_error(filename, linenumber);
			file_print("String: ", stderr);
			file_print(store, stderr);
			file_print(" exceeds max string size\n", stderr);
			exit(EXIT_FAILURE);
		}
	} while(ch != c);

	return store;
}

struct Token* Tokenize_Line(struct Token* head, char* filename)
{
	int c;
	struct Token* p;
	linenumber = 1;

	do
	{
restart:
		c = fgetc(source_file);

		if(in_set(c, ";#"))
		{
			purge_lineComment();
			head = append_newline(head, filename);
			goto restart;
		}

		if(in_set(c, "\t "))
		{
			goto restart;
		}

		if('\n' == c)
		{
			head = append_newline(head, filename);
			goto restart;
		}

		if(EOF == c)
		{
			head = append_newline(head, filename);
			goto done;
		}

		p = newToken(filename, linenumber);
		p->next = head;
		if(in_set(c, "'\""))
		{
			p->Text = store_string(c, filename);
			p->type = STR;
		}
		else
		{
			p = store_atom(p, c, filename);
		}

		head = p;
	} while(TRUE);
done:
	return head;
}

void setExpression(struct Token* p, char *c, char *Exp)
{
	struct Token* i;
	for(i = p; NULL != i; i = i->next)
	{
		/* Leave macros alone */
		if(MACRO == i->type)
		{
			if(match(i->Text, c))
			{
				line_error(i->filename, i->linenumber);
				file_print("Multiple definitions for macro ", stderr);
				file_print(c, stderr);
				file_print("\n", stderr);
				exit(EXIT_FAILURE);
			}
			continue;
		}
		else if(match(i->Text, c))
		{ /* Only if there is an exact match replace */
			i->Expression = Exp;
		}
	}
}

void identify_macros(struct Token* p)
{
	struct Token* i;
	for(i = p; NULL != i; i = i->next)
	{
		if(match(i->Text, "DEFINE"))
		{
			i->type = MACRO;
			i->Text = i->next->Text;
			if(STR == i->next->next->type)
			{
				i->Expression = i->next->next->Text + 1;
			}
			else
			{
				i->Expression = i->next->next->Text;
			}
			i->next = i->next->next->next;
		}
	}
}

void line_macro(struct Token* p)
{
	struct Token* i;
	for(i = p; NULL != i; i = i->next)
	{
		if(MACRO == i->type)
		{
			setExpression(i->next, i->Text, i->Expression);
		}
	}
}

void hexify_string(struct Token* p)
{
	char* table = "0123456789ABCDEF";
	int i = ((string_length(p->Text + 1)/4) + 1) * 8;

	char* d = calloc(max_string, sizeof(char));
	p->Expression = d;

	while(0 < i)
	{
		i = i - 1;
		d[i] = '0';
	}

	while( i < max_string)
	{
		if(0 == p->Text[i+1])
		{
			i = max_string;
		}
		else
		{
			d[2*i]  = table[p->Text[i+1] / 16];
			d[2*i + 1] = table[p->Text[i+1] % 16];
			i = i + 1;
		}
	}
}

void process_string(struct Token* p)
{
	struct Token* i;
	for(i = p; NULL != i; i = i->next)
	{
		if(STR == i->type)
		{
			if('\'' == i->Text[0])
			{
				i->Expression = i->Text + 1;
			}
			else if('"' == i->Text[0])
			{
				hexify_string(i);
			}
		}
	}
}

char* pad_nulls(int size, char* nil)
{
	if(0 == size) return nil;
	size = size * 2;

	char* s = calloc(size + 1, sizeof(char));

	int i = 0;
	while(i < size)
	{
		s[i] = '0';
		i = i + 1;
	}

	return s;
}

void preserve_other(struct Token* p)
{
	struct Token* i;
	char c;
	for(i = p; NULL != i; i = i->next)
	{
		if((NULL == i->Expression) && !(i->type & MACRO))
		{
			c = i->Text[0];

			if(in_set(c, "!@$~%&:^"))
			{
				i->Expression = i->Text;
			}
			else if('<' == c)
			{
				i->Expression = pad_nulls(numerate_string(i->Text + 1), i->Text);
			}
			else
			{
				line_error(i->filename, i->linenumber);
				file_print("Received invalid other; ", stderr);
				file_print(i->Text, stderr);
				file_print("\n", stderr);
				exit(EXIT_FAILURE);
			}
		}
	}
}

void bound_values(int displacement, int number_of_bytes, int low, int high)
{
	if((high < displacement) || (displacement < low))
	{
		file_print("A displacement of ", stderr);
		file_print(numerate_number(displacement), stderr);
		file_print(" does not fit in ", stderr);
		file_print(numerate_number(number_of_bytes), stderr);
		file_print(" bytes\n", stderr);
		exit(EXIT_FAILURE);
	}
}

void range_check(int displacement, int number_of_bytes)
{
	if(4 == number_of_bytes) return;
	else if(3 == number_of_bytes)
	{
		bound_values(displacement, number_of_bytes, -8388608, 16777216);
		return;
	}
	else if(2 == number_of_bytes)
	{
		bound_values(displacement, number_of_bytes, -32768, 65535);
		return;
	}
	else if(1 == number_of_bytes)
	{
		bound_values(displacement, number_of_bytes, -128, 255);
		return;
	}

	file_print("Received an invalid number of bytes in range_check\n", stderr);
	exit(EXIT_FAILURE);
}

void reverseBitOrder(char* c)
{
	if(NULL == c) return;
	if(0 == c[1]) return;
	int hold = c[0];

	if(16 == ByteMode)
	{
		c[0] = c[1];
		c[1] = hold;
		reverseBitOrder(c+2);
	}
	else if(8 == ByteMode)
	{
		c[0] = c[2];
		c[2] = hold;
		reverseBitOrder(c+3);
	}
	else if(2 == ByteMode)
	{
		c[0] = c[7];
		c[7] = hold;
		hold = c[1];
		c[1] = c[6];
		c[6] = hold;
		hold = c[2];
		c[2] = c[5];
		c[5] = hold;
		hold = c[3];
		c[3] = c[4];
		c[4] = hold;
		reverseBitOrder(c+8);
	}
}

void LittleEndian(char* start)
{
	char* end = start;
	char* c = start;
	while(0 != end[0]) end = end + 1;
	int hold;
	for(end = end - 1; start < end; start = start + 1)
	{
		hold = start[0];
		start[0] = end[0];
		end[0] = hold;
		end = end - 1;
	}

	if(BigBitEndian) reverseBitOrder(c);
}

int stringify(char* s, int digits, int divisor, int value, int shift)
{
	int i = value;
	if(digits > 1)
	{
		i = stringify(s+1, (digits - 1), divisor, value, shift);
	}
	s[0] = hex2char(i & (divisor - 1));
	return (i >> shift);
}

char* express_number(int value, char c)
{
	char* ch = calloc(42, sizeof(char));
	int size;
	int number_of_bytes;
	int shift;
	if('!' == c)
	{
		number_of_bytes = 1;
		value = value & 0xFF;
	}
	else if('@' == c)
	{
		number_of_bytes = 2;
		value = value & 0xFFFF;
	}
	else if('~' == c)
	{
		number_of_bytes = 3;
		value = value & 0xFFFFFF;
	}
	else if('%' == c)
	{
		number_of_bytes = 4;
		value = value & 0xFFFFFFFF;
	}
	else
	{
		file_print("Given symbol ", stderr);
		fputc(c, stderr);
		file_print(" to express immediate value ", stderr);
		file_print(numerate_number(value), stderr);
		fputc('\n', stderr);
		exit(EXIT_FAILURE);
	}

	range_check(value, number_of_bytes);

	if(16 == ByteMode)
	{
		size = number_of_bytes * 2;
		shift = 4;
	}
	else if(8 == ByteMode)
	{
		size = number_of_bytes * 3;
		shift = 3;
	}
	else if(2 == ByteMode)
	{
		size = number_of_bytes * 8;
		shift = 1;
	}
	else
	{
		file_print("Got invalid ByteMode in express_number\n", stderr);
		exit(EXIT_FAILURE);
	}

	stringify(ch, size, ByteMode, value, shift);

	if(!BigEndian) LittleEndian(ch);
	else if(!BigBitEndian) reverseBitOrder(ch);
	return ch;
}

void eval_immediates(struct Token* p)
{
	struct Token* i;
	int value;
	for(i = p; NULL != i; i = i->next)
	{
		if(MACRO == i->type) continue;
		else if(NEWLINE == i->type) continue;
		else if('<' == i->Text[0]) continue;
		else if(NULL == i->Expression)
		{
			if((1 == Architecture) || (2 == Architecture) || (40 == Architecture))
			{
				value = numerate_string(i->Text + 1);
				if(('0' == i->Text[1]) || (0 != value))
				{
					i->Expression = express_number(value, i->Text[0]);
				}
			}
			else if(0 == Architecture)
			{
				value = numerate_string(i->Text);
				if(('0' == i->Text[0]) || (0 != value))
				{
					i->Expression = express_number(value, '@');
				}
			}
			else
			{
				file_print("Unknown architecture received in eval_immediates\n", stderr);
				exit(EXIT_FAILURE);
			}
		}
	}
}

void print_hex(struct Token* p)
{
	struct Token* i;
	for(i = p; NULL != i; i = i->next)
	{
		if(NEWLINE == i->type)
		{
			if(NULL == i->next) fputc('\n', destination_file);
			else if((NEWLINE != i->next->type) && (MACRO != i->next->type)) fputc('\n', destination_file);
		}
		else if(i->type != MACRO)
		{
			file_print(i->Expression, destination_file);
			if(NEWLINE != i->next->type) fputc(' ', destination_file);
		}
	}
}

/* Standard C main program */
int main(int argc, char **argv)
{
	BigEndian = TRUE;
	struct Token* head = NULL;
	Architecture = 0;
	destination_file = stdout;
	BigBitEndian = TRUE;
	ByteMode = 16;
	char* filename;
	char* arch;

	int option_index = 1;
	while(option_index <= argc)
	{
		if(NULL == argv[option_index])
		{
			option_index = option_index + 1;
		}
		else if(match(argv[option_index], "--BigEndian"))
		{
			BigEndian = TRUE;
			option_index = option_index + 1;
		}
		else if(match(argv[option_index], "--LittleEndian"))
		{
			BigEndian = FALSE;
			option_index = option_index + 1;
		}
		else if(match(argv[option_index], "-A") || match(argv[option_index], "--architecture"))
		{
			arch = argv[option_index + 1];
			if(match("knight-native", arch) || match("knight-posix", arch)) Architecture = 0;
			else if(match("x86", arch)) Architecture = 1;
			else if(match("amd64", arch)) Architecture = 2;
			else if(match("armv7l", arch)) Architecture = 40;
			else
			{
				file_print("Unknown architecture: ", stderr);
				file_print(arch, stderr);
				file_print(" know values are: knight-native, knight-posix, x86, amd64 and armv7l", stderr);
			}
			option_index = option_index + 2;
		}
		else if(match(argv[option_index], "-b") || match(argv[option_index], "--binary"))
		{
			ByteMode = 2;
			option_index = option_index + 1;
		}
		else if(match(argv[option_index], "-h") || match(argv[option_index], "--help"))
		{
			file_print("Usage: ", stderr);
			file_print(argv[0], stderr);
			file_print(" -f FILENAME1 {-f FILENAME2} (--BigEndian|--LittleEndian) ", stderr);
			file_print("[--architecture name]\nArchitectures: knight-native, knight-posix, x86, amd64 and armv7\n", stderr);
			exit(EXIT_SUCCESS);
		}
		else if(match(argv[option_index], "-f") || match(argv[option_index], "--file"))
		{
			filename = argv[option_index + 1];
			source_file = fopen(filename, "r");

			if(NULL == source_file)
			{
				file_print("The file: ", stderr);
				file_print(argv[option_index + 1], stderr);
				file_print(" can not be opened!\n", stderr);
				exit(EXIT_FAILURE);
			}

			head = Tokenize_Line(head, filename);
			option_index = option_index + 2;
		}
		else if(match(argv[option_index], "-o") || match(argv[option_index], "--output"))
		{
			destination_file = fopen(argv[option_index + 1], "w");

			if(NULL == destination_file)
			{
				file_print("The file: ", stderr);
				file_print(argv[option_index + 1], stderr);
				file_print(" can not be opened!\n", stderr);
				exit(EXIT_FAILURE);
			}
			option_index = option_index + 2;
		}
		else if(match(argv[option_index], "-O") || match(argv[option_index], "--octal"))
		{
			ByteMode = 8;
			option_index = option_index + 1;
		}
		else if(match(argv[option_index], "-V") || match(argv[option_index], "--version"))
		{
			file_print("M1 0.3\n", stdout);
			exit(EXIT_SUCCESS);
		}
		else
		{
			file_print("Unknown option\n", stderr);
			exit(EXIT_FAILURE);
		}
	}

	if(NULL == head)
	{
		file_print("Either no input files were given or they were empty\n", stderr);
		exit(EXIT_FAILURE);
	}

	head = reverse_list(head);
	identify_macros(head);
	line_macro(head);
	process_string(head);
	eval_immediates(head);
	preserve_other(head);
	print_hex(head);

	return EXIT_SUCCESS;
}
