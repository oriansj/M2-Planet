/* Copyright (C) 2016 Jeremiah Orians
 * Copyright (C) 2020 deesix <deesix@tuta.io>
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
#include<string.h>
#include"cc.h"

/* The core functions */
void initialize_types();
struct token_list* read_all_tokens(FILE* a, struct token_list* current, char* filename);
struct token_list* reverse_list(struct token_list* head);
void program();
void recursive_output(struct token_list* i, FILE* out);

int main(int argc, char** argv)
{
	hold_string = calloc(MAX_STRING, sizeof(char));
	require(NULL != hold_string, "Impossible Exhustion has occured\n");
	int DEBUG = FALSE;
	FILE* in = stdin;
	FILE* destination_file = stdout;
	Architecture = KNIGHT_NATIVE; /* Assume Knight-native */
	char* arch;

	int i = 1;
	while(i <= argc)
	{
		if(NULL == argv[i])
		{
			i = i + 1;
		}
		else if(match(argv[i], "-f") || match(argv[i], "--file"))
		{
			char* name = argv[i + 1];
			in = fopen(name, "r");
			if(NULL == in)
			{
				file_print("Unable to open for reading file: ", stderr);
				file_print(name, stderr);
				file_print("\n Aborting to avoid problems\n", stderr);
				exit(EXIT_FAILURE);
			}
			global_token = read_all_tokens(in, global_token, name);
			i = i + 2;
		}
		else if(match(argv[i], "-o") || match(argv[i], "--output"))
		{
			destination_file = fopen(argv[i + 1], "w");
			if(NULL == destination_file)
			{
				file_print("Unable to open for writing file: ", stderr);
				file_print(argv[i + 1], stderr);
				file_print("\n Aborting to avoid problems\n", stderr);
				exit(EXIT_FAILURE);
			}
			i = i + 2;
		}
		else if(match(argv[i], "-A") || match(argv[i], "--architecture"))
		{
			arch = argv[i + 1];
			if(match("knight-native", arch)) Architecture = KNIGHT_NATIVE;
			else if(match("knight-posix", arch)) Architecture = KNIGHT_POSIX;
			else if(match("x86", arch)) Architecture = X86;
			else if(match("amd64", arch)) Architecture = AMD64;
			else if(match("armv7l", arch)) Architecture = ARMV7L;
			else if(match("aarch64", arch)) Architecture = AARCH64;
			else
			{
				file_print("Unknown architecture: ", stderr);
				file_print(arch, stderr);
				file_print(" know values are: knight-native, knight-posix, x86, amd64, armv7l and aarch64", stderr);
				exit(EXIT_FAILURE);
			}
			i = i + 2;
		}
		else if(match(argv[i], "-g") || match(argv[i], "--debug"))
		{
			DEBUG = TRUE;
			i = i + 1;
		}
		else if(match(argv[i], "-h") || match(argv[i], "--help"))
		{
			file_print(" -f input file\n -o output file\n --help for this message\n --version for file version\n", stdout);
			exit(EXIT_SUCCESS);
		}
		else if(match(argv[i], "-V") || match(argv[i], "--version"))
		{
			file_print("M2-Planet v1.5.0\n", stderr);
			exit(EXIT_SUCCESS);
		}
		else
		{
			file_print("UNKNOWN ARGUMENT\n", stdout);
			exit(EXIT_FAILURE);
		}
	}

	/* Deal with special case of wanting to read from standard input */
	if(stdin == in)
	{
		global_token = read_all_tokens(in, global_token, "STDIN");
	}

	if(NULL == global_token)
	{
		file_print("Either no input files were given or they were empty\n", stderr);
		exit(EXIT_FAILURE);
	}
	global_token = reverse_list(global_token);

	initialize_types();
	reset_hold_string();
	output_list = NULL;
	program();

	/* Output the program we have compiled */
	file_print("\n# Core program\n", destination_file);
	recursive_output(output_list, destination_file);
	if(KNIGHT_NATIVE == Architecture) file_print("\n", destination_file);
	else if(DEBUG) file_print("\n:ELF_data\n", destination_file);
	file_print("\n# Program global variables\n", destination_file);
	recursive_output(globals_list, destination_file);
	file_print("\n# Program strings\n", destination_file);
	recursive_output(strings_list, destination_file);
	if(KNIGHT_NATIVE == Architecture) file_print("\n:STACK\n", destination_file);
	else if(!DEBUG) file_print("\n:ELF_end\n", destination_file);
	return EXIT_SUCCESS;
}
