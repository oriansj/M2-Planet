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
void line_error();

/* Initialize default types */
void initialize_types()
{
	/* Define void */
	global_types = calloc(1, sizeof(struct type));
	global_types->name = "void";
	global_types->size = 4;
	global_types->type = global_types;
	/* void* has the same properties as void */
	global_types->indirect = global_types;

	/* Define int */
	struct type* a = calloc(1, sizeof(struct type));
	a->name = "int";
	a->size = 4;
	/* int* has the same properties as int */
	a->indirect = a;
	a->type = a;

	/* Define char* */
	struct type* b = calloc(1, sizeof(struct type));
	b->name = "char*";
	b->size = 4;
	b->type = b;

	/* Define char */
	struct type* c = calloc(1, sizeof(struct type));
	c->name = "char";
	c->size = 1;
	c->type = c;

	/* char** is char */
	c->indirect = b;
	b->indirect = c;

	/* Define FILE */
	struct type* d = calloc(1, sizeof(struct type));
	d->name = "FILE";
	d->size = 4;
	d->type = d;
	/* FILE* has the same properties as FILE */
	d->indirect = d;

	/* Define FUNCTION */
	struct type* e = calloc(1, sizeof(struct type));
	e->name = "FUNCTION";
	e->size = 4;
	e->type = e;
	/* FUNCTION* has the same properties as FUNCTION */
	e->indirect = e;

	/* Finalize type list */
	d->next = e;
	c->next = d;
	a->next = c;
	global_types->next = a;
}

struct type* lookup_type(char* s)
{
	struct type* i;
	for(i = global_types; NULL != i; i = i->next)
	{
		if(match(i->name, s))
		{
			return i;
		}
	}
	return NULL;
}

struct type* type_name();
void require_match(char* message, char* required);

int member_size;
struct type* build_member(struct type* last, int offset)
{
	struct type* member_type = type_name();
	struct type* i = calloc(1, sizeof(struct type));
	i->name = global_token->s;
	i->members = last;
	i->size = member_type->size;
	member_size = member_type->size;
	i->type = member_type;
	i->offset = offset;
	return i;
}

struct type* build_union(struct type* last, int offset)
{
	int size = 0;
	global_token = global_token->next;
	require_match("ERROR in build_union\nMissing {\n", "{");
	while('}' != global_token->s[0])
	{
		last = build_member(last, offset);
		if(member_size > size)
		{
			size = member_size;
		}
		global_token = global_token->next;
		require_match("ERROR in build_union\nMissing ;\n", ";");
	}
	member_size = size;
	return last;
}

void create_struct()
{
	int offset = 0;
	struct type* head = calloc(1, sizeof(struct type));
	struct type* i = calloc(1, sizeof(struct type));
	head->name = global_token->s;
	i->name = global_token->s;
	head->indirect = i;
	i->indirect = head;
	head->next = global_types;
	global_types = head;
	global_token = global_token->next;
	i->size = 4;
	require_match("ERROR in create_struct\x0A Missing {\x0A", "{");
	struct type* last = NULL;
	while('}' != global_token->s[0])
	{
		if(match(global_token->s, "union"))
		{
			last = build_union(last, offset);
		}
		else
		{
			last = build_member(last, offset);
		}
		offset = offset + member_size;
		global_token = global_token->next;
		require_match("ERROR in create_struct\x0A Missing ;\x0A", ";");
	}

	global_token = global_token->next;
	require_match("ERROR in create_struct\x0A Missing ;\x0A", ";");

	head->size = offset;
	head->members = last;
	head->indirect->members = last;
}


/*
 * type-name:
 *     char *
 *     int
 *     struct
 *     FILE
 *     void
 */
struct type* type_name()
{
	int structure = FALSE;

	if(match("struct", global_token->s))
	{
		structure = TRUE;
		global_token = global_token->next;
	}

	struct type* ret = lookup_type(global_token->s);

	if(NULL == ret && !structure)
	{
		file_print("Unknown type ", stderr);
		file_print(global_token->s, stderr);
		file_print("\x0A", stderr);
		line_error();
		exit(EXIT_FAILURE);
	}
	else if(NULL == ret)
	{
		create_struct();
		return NULL;
	}

	global_token = global_token->next;

	while(global_token->s[0] == '*')
	{
		ret = ret->indirect;
		global_token = global_token->next;
	}

	return ret;
}
