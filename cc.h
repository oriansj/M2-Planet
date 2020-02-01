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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// CONSTANT MAX_STRING 4096
#define MAX_STRING 4096
// CONSTANT FALSE 0
#define FALSE 0
// CONSTANT TRUE 1
#define TRUE 1

// CONSTANT KNIGHT_NATIVE 0
#define KNIGHT_NATIVE 0
// CONSTANT KNIGHT_POSIX 1
#define KNIGHT_POSIX 1
// CONSTANT X86 2
#define X86 2
// CONSTANT AMD64 3
#define AMD64 3
// CONSTANT ARMV7L 4
#define ARMV7L 4
// CONSTANT AARCH64 5
#define AARCH64 5


char* copy_string(char* target, char* source);
int in_set(int c, char* s);
int match(char* a, char* b);
void file_print(char* s, FILE* f);
void require(int bool, char* error);
void reset_hold_string();


struct type
{
	struct type* next;
	int size;
	int offset;
	int is_signed;
	struct type* indirect;
	struct type* members;
	struct type* type;
	char* name;
};

struct token_list
{
	struct token_list* next;
	union
	{
		struct token_list* locals;
		struct token_list* prev;
	};
	char* s;
	union
	{
		struct type* type;
		char* filename;
	};
	union
	{
		struct token_list* arguments;
		int depth;
		int linenumber;
	};
};

/* What types we have */
struct type* global_types;
struct type* prim_types;

/* What we are currently working on */
struct token_list* global_token;

/* Output reorder collections*/
struct token_list* output_list;
struct token_list* strings_list;
struct token_list* globals_list;

/* Make our string collection more efficient */
char* hold_string;
int string_index;

/* Our Target Architecture */
int Architecture;
int register_size;
