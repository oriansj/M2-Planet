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

#ifndef LISP_H
#define LISP_H

#include "gcc_req.h"
#include <bootstrappable.h>

enum
{
	FREE = 1,
	MARKED = 2,
	INT = 4,
	SYM = 8,
	CONS = 16,
	PROC = 32,
	PRIMOP = 64,
	CHAR = 128,
	STRING = 256,
};

enum
{
	FALSE = 0,
	TRUE = 1,
};

struct cell
{
	int type;
	union
	{
		struct cell* car;
		int value;
		char* string;
		FUNCTION* function;
	};
	struct cell* cdr;
	struct cell* env;
};

enum
{
	MAX_STRING = 4096,
};

/* Common functions */
struct cell* make_cons(struct cell* a, struct cell* b);
int strtoint(char *a);
char* int2str(int x, int base, int signed_p);
int match(char* a, char* b);

/* Global objects */
struct cell *all_symbols;
struct cell *top_env;
struct cell *nil;
struct cell *tee;
struct cell *quote;
struct cell *s_if;
struct cell *s_lambda;
struct cell *s_define;
struct cell *s_setb;
struct cell *s_cond;
struct cell *s_begin;
struct cell *s_let;
struct cell *s_while;
struct cell *current;
FILE* input;
FILE* file_output;
FILE* console_output;
int echo;
int left_to_take;

#endif /* LISP_H */

