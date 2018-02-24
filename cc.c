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
#include<stdio.h>
#include<string.h>
#include"cc.h"

/* The core functions */
void initialize_types();
struct token_list* read_all_tokens(FILE* a, struct token_list* current);
struct token_list* reverse_list(struct token_list* head);
struct token_list* program(struct token_list* out);
void recursive_output(struct token_list* i, FILE* out);
int match(char* a, char* b);
void file_print(char* s, FILE* f);
char* parse_string(char* string);

void test0(struct token_list* l, FILE* o)
{
	while(NULL != l)
	{
		if(l->s[0] == 34)
		{
			file_print(parse_string(l->s), o);
		}
		else
		{
			file_print(l->s, o);
		}
		fputc(10, o);
		l = l->next;
	}
}

int main(int argc, char** argv)
{
	FILE* in = stdin;
	FILE* destination_file = stdout;
	int i = 1;
	while(i <= argc)
	{
		if(NULL == argv[i])
		{
			i = i + 1;
		}
		else if(match(argv[i], "-f"))
		{
			in = fopen(argv[i + 1], "r");
			if(NULL == in)
			{
				file_print("Unable to open for reading file: ", stderr);
				file_print(argv[i + 1], stderr);
				file_print("\x0A Aborting to avoid problems\x0A", stderr);
				exit(EXIT_FAILURE);
			}
			global_token = read_all_tokens(in, global_token);
			i = i + 2;
		}
		else if(match(argv[i], "-o"))
		{
			destination_file = fopen(argv[i + 1], "w");
			if(NULL == destination_file)
			{
				file_print("Unable to open for writing file: ", stderr);
				file_print(argv[i + 1], stderr);
				file_print("\x0A Aborting to avoid problems\x0A", stderr);
				exit(EXIT_FAILURE);
			}
			i = i + 2;
		}
		else if(match(argv[i], "--help"))
		{
			file_print(" -f input file\x0A -o output file\x0A --help for this message\x0A --version for file version\x0A", stdout);
			exit(EXIT_SUCCESS);
		}
		else if(match(argv[i], "--version"))
		{
			file_print("Basic test version 0.0.0.1a\x0A", stderr);
			exit(EXIT_SUCCESS);
		}
		else
		{
			file_print("UNKNOWN ARGUMENT\x0A", stdout);
			exit(EXIT_FAILURE);
		}
	}

	if(NULL == global_token)
	{
		file_print("Either no input files were given or they were empty\n", stderr);
		exit(EXIT_FAILURE);
	}
	global_token = reverse_list(global_token);

	initialize_types();
	struct token_list* output_list = program(NULL);

	/* Output the program we have compiled */
	file_print("\n# Core program\n\n", destination_file);
	recursive_output(output_list, destination_file);
	file_print("\n# Program global variables\n\n", destination_file);
	recursive_output(globals_list, destination_file);
	file_print("\n# Program strings\n\n", destination_file);
	recursive_output(strings_list, destination_file);
	file_print("\n:ELF_end\n", destination_file);
	return EXIT_SUCCESS;
}
