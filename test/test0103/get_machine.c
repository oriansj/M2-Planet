/* -*- c-file-style: "linux";indent-tabs-mode:t -*- */
/* Copyright (C) 2017 Jeremiah Orians
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
#include <sys/utsname.h>
void file_print(char* s, FILE* f);
int match(char* a, char* b);

#define TRUE 1
//CONSTANT TRUE 1
#define FALSE 0
//CONSTANT FALSE 0

/* Standard C main program */
int main(int argc, char **argv)
{
	int exact = FALSE;
	int override = FALSE;
	char* override_string;
	int option_index = 1;
	while(option_index <= argc)
	{
		if(NULL == argv[option_index])
		{
			option_index = option_index + 1;
		}
		else if(match(argv[option_index], "--exact"))
		{
			exact = TRUE;
			option_index = option_index + 1;
		}
		else if(match(argv[option_index], "--override"))
		{
			override = TRUE;
			override_string = argv[option_index + 1];
			option_index = option_index + 2;
		}
		else if(match(argv[option_index], "-V") || match(argv[option_index], "--version"))
		{
			file_print("get_machine 0.1\n", stdout);
			exit(EXIT_SUCCESS);
		}
		else
		{
			file_print("Unknown option\n", stderr);
			exit(EXIT_FAILURE);
		}
	}

	struct utsname* unameData = calloc(1, sizeof(struct utsname));
	uname(unameData);
	if(override) file_print(override_string, stdout);
	else if(!exact)
	{
		if(match("i386", unameData->machine) ||
		   match("i486", unameData->machine) ||
		   match("i586", unameData->machine) ||
		   match("i686", unameData->machine) ||
		   match("i686-pae", unameData->machine)) file_print("x86", stdout);
		else if(match("x86_64", unameData->machine)) file_print("amd64", stdout);
		else file_print(unameData->machine, stdout);
	}
	else file_print(unameData->machine, stdout);
	file_print("\n", stdout);
	return EXIT_SUCCESS;
}
