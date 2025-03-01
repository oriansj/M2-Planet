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

// CONSTANT FALSE 0
#define FALSE 0
// CONSTANT TRUE 1
#define TRUE 1

// CONSTANT KNIGHT_NATIVE 1
#define KNIGHT_NATIVE 1
// CONSTANT KNIGHT_POSIX 2
#define KNIGHT_POSIX 2
// CONSTANT X86 3
#define X86 3
// CONSTANT AMD64 4
#define AMD64 4
// CONSTANT ARMV7L 5
#define ARMV7L 5
// CONSTANT AARCH64 6
#define AARCH64 6
// CONSTANT RISCV32 7
#define RISCV32 7
// CONSTANT RISCV64 8
#define RISCV64 8

// CONSTANT NO_STRUCT_DEFINITION 0
#define NO_STRUCT_DEFINITION 0

int copy_string(char* target, char* source, int max);
int string_length(char* a);
int in_set(int c, char* s);
int match(char* a, char* b);
void require(int bool, char* error);
void reset_hold_string(void);
char* int2str(int x, int base, int signed_p);

#define REGISTER_ZERO 0
// CONSTANT REGISTER_ZERO 0
#define REGISTER_ONE 1
// CONSTANT REGISTER_ONE 1
#define REGISTER_TEMP 2
// CONSTANT REGISTER_TEMP 2
#define REGISTER_BASE 3
// CONSTANT REGISTER_BASE 3
/* AARCH64 and RISCV32/RISCV64 have return pointers. */
#define REGISTER_RETURN 4
// CONSTANT REGISTER_RETURN 4

void emit_push(int, char*);
void emit_pop(int, char*);

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

/* TLO = Token List Option */
// CONSTANT TLO_LOCAL_ARRAY 1
#define TLO_LOCAL_ARRAY 1

struct token_list
{
	struct token_list* next;
	struct token_list* locals;
	struct token_list* prev;
	char* s;
	struct type* type;
	char* filename;
	struct token_list* arguments;
	int depth;
	int linenumber;
	int array_modifier;
	int options;
};

struct case_list
{
	struct case_list* next;
	char* value;
};

struct static_variable_list
{
	struct static_variable_list* next;
	char* local_variable_name;
	struct token_list* global_variable;
};

struct token_list* sym_declare(char *s, struct type* t, struct token_list* list);

#include "cc_globals.h"
