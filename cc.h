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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_STRING 4096
// CONSTANT MAX_STRING 4096
#define FALSE 0
// CONSTANT FALSE 0
#define TRUE 1
// CONSTANT TRUE 1
// CONSTANT NULL 0
// CONSTANT EXIT_FAILURE 1
// CONSTANT EXIT_SUCCESS 0
// CONSTANT stdin 0
// CONSTANT stdout 1
// CONSTANT stderr 2

void file_print(char* s, FILE* f);

struct type
{
	struct type* next;
	int size;
	int offset;
	struct type* indirect;
	struct type* members;
	struct type* type;
	char* name;
};

struct token_list
{
	struct token_list* next;
	struct token_list* prev;
	struct token_list* entry;
	char* s;
	struct type* type;
	struct token_list* arguments;
	struct token_list* locals;
	int temps;
};

/* What types we have */
struct type* global_types;

/* What we are currently working on */
struct token_list* global_token;

/* Output reorder collections*/
struct token_list* strings_list;
struct token_list* globals_list;
