/* Copyright (C) 2016 Jeremiah Orians
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

#include "cc.h"
#include <stdint.h>

/* Imported functions */
int numerate_string(char *a);
void line_error();
void require(int bool, char* error);

/* Initialize default types */
void initialize_types()
{
	if(AMD64 == Architecture) register_size = 8;
	else register_size = 4;

	/* Define void */
	global_types = calloc(1, sizeof(struct type));
	global_types->name = "void";
	global_types->is_signed = FALSE;
	global_types->size = register_size;
	global_types->type = global_types;
	/* void* has the same properties as void */
	global_types->indirect = global_types;

	/* Define UNis_signed LONG */
	struct type* a = calloc(1, sizeof(struct type));
	a->name = "SCM";
	a->is_signed = FALSE;
	a->size = register_size;
	a->indirect = a;
	a->type = a;

	/* Define LONG */
	struct type* b = calloc(1, sizeof(struct type));
	b->name = "long";
	b->is_signed = TRUE;
	b->size = register_size;
	b->indirect = b;
	b->type = b;

	/* Define UNSIGNED */
	struct type* c = calloc(1, sizeof(struct type));
	c->name = "unsigned";
	c->is_signed = FALSE;
	c->size = register_size;
	c->type = c;
	/* unsigned* has the same properties as unsigned */
	c->indirect = c;

	/* Define int */
	struct type* d = calloc(1, sizeof(struct type));
	d->name = "int";
	d->is_signed = TRUE;
	d->size = register_size;
	/* int* has the same properties as int */
	d->indirect = d;
	d->type = d;

	/* Define char* */
	struct type* e = calloc(1, sizeof(struct type));
	e->name = "char*";
	e->is_signed = FALSE;
	e->size = register_size;
	e->type = e;

	/* Define char */
	struct type* f = calloc(1, sizeof(struct type));
	f->name = "char";
	f->is_signed = FALSE;
	f->size = 1;
	f->type = f;

	/* Define char** */
	struct type* g = calloc(1, sizeof(struct type));
	g->name = "char**";
	g->is_signed = FALSE;
	g->size = register_size;
	g->type = e;
	g->indirect = g;

	/*fix up indirects for chars */
	f->indirect = e;
	e->indirect = g;

	/* Define FILE */
	struct type* h = calloc(1, sizeof(struct type));
	h->name = "FILE";
	h->is_signed = FALSE;
	h->size = register_size;
	h->type = h;
	/* FILE* has the same properties as FILE */
	h->indirect = h;

	/* Define FUNCTION */
	struct type* i = calloc(1, sizeof(struct type));
	i->name = "FUNCTION";
	i->is_signed = FALSE;
	i->size = register_size;
	i->type = i;
	/* FUNCTION* has the same properties as FUNCTION */
	i->indirect = i;

	/* Primitives mes.c wanted */
	struct type* j = calloc(1, sizeof(struct type));
	j->name = "size_t";
	j->is_signed = FALSE;
	j->size = register_size;
	j->indirect = j;

	struct type* k = calloc(1, sizeof(struct type));
	k->name = "ssize_t";
	k->is_signed = FALSE;
	k->size = register_size;
	k->indirect = k;

	/* Finalize type list */
	j->next = k;
	i->next = j;
	h->next = i;
	g->next = h;
	f->next = g;
	d->next = f;
	c->next = d;
	b->next = c;
	a->next = b;
	global_types->next = a;
	prim_types = global_types;
}

struct type* lookup_type(char* s, struct type* start)
{
	struct type* i;
	for(i = start; NULL != i; i = i->next)
	{
		if(match(i->name, s))
		{
			return i;
		}
	}
	return NULL;
}

struct type* lookup_member(struct type* parent, char* name)
{
	struct type* i;
	require(NULL != parent, "Not a valid struct type\n");
	for(i = parent->members; NULL != i; i = i->members)
	{
		if(match(i->name, name)) return i;
	}

	file_print("ERROR in lookup_member ", stderr);
	file_print(parent->name, stderr);
	file_print("->", stderr);
	file_print(global_token->s, stderr);
	file_print(" does not exist\n", stderr);
	line_error();
	file_print("\n", stderr);
	exit(EXIT_FAILURE);
}

struct type* type_name();
void require_match(char* message, char* required);

int member_size;
struct type* build_member(struct type* last, int offset)
{
	struct type* i = calloc(1, sizeof(struct type));
	i->members = last;
	i->offset = offset;

	struct type* member_type = type_name();
	require(NULL != member_type, "struct member type can not be invalid\n");
	i->type = member_type;
	i->name = global_token->s;
	global_token = global_token->next;
	require(NULL != global_token, "struct member can not be EOF terminated\n");

	/* Check to see if array */
	if(match( "[", global_token->s))
	{
		global_token = global_token->next;
		require(NULL != global_token, "struct member arrays can not be EOF sized\n");
		i->size = member_type->type->size * numerate_string(global_token->s);
		if(0 == i->size)
		{
			file_print("Struct only supports [num] form\n", stderr);
			exit(EXIT_FAILURE);
		}
		global_token = global_token->next;
		require_match("Struct only supports [num] form\n", "]");
	}
	else
	{
		i->size = member_type->size;
	}
	member_size = i->size;

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
		require_match("ERROR in build_union\nMissing ;\n", ";");
		require(NULL != global_token, "Unterminated union\n");
	}
	member_size = size;
	global_token = global_token->next;
	return last;
}

void create_struct()
{
	int offset = 0;
	member_size = 0;
	struct type* head = calloc(1, sizeof(struct type));
	struct type* i = calloc(1, sizeof(struct type));
	head->name = global_token->s;
	head->type = head;
	i->name = global_token->s;
	i->type = i;
	head->indirect = i;
	i->indirect = head;
	head->next = global_types;
	global_types = head;
	global_token = global_token->next;
	i->size = register_size;
	require_match("ERROR in create_struct\n Missing {\n", "{");
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
		require_match("ERROR in create_struct\n Missing ;\n", ";");
		require(NULL != global_token, "Unterminated struct\n");
	}

	global_token = global_token->next;
	require_match("ERROR in create_struct\n Missing ;\n", ";");

	head->size = offset;
	head->members = last;
	i->members = last;
}


struct type* type_name()
{
	struct type* ret;

	require(NULL != global_token, "Recieved EOF instead of type name\n");
	if(match("struct", global_token->s))
	{
		global_token = global_token->next;
		require(NULL != global_token, "structs can not have a EOF type name\n");
		ret = lookup_type(global_token->s, global_types);
		if(NULL == ret)
		{
			create_struct();
			return NULL;
		}
	}
	else
	{
		ret = lookup_type(global_token->s, global_types);
		if(NULL == ret)
		{
			file_print("Unknown type ", stderr);
			file_print(global_token->s, stderr);
			file_print("\n", stderr);
			line_error();
			exit(EXIT_FAILURE);
		}
	}

	global_token = global_token->next;
	require(NULL != global_token, "unfinished type definition\n");

	if(match("const", global_token->s))
	{
		global_token = global_token->next;
		require(NULL != global_token, "unfinished type definition in const\n");
	}

	while(global_token->s[0] == '*')
	{
		ret = ret->indirect;
		global_token = global_token->next;
		require(NULL != global_token, "unfinished type definition in indirection\n");
	}

	return ret;
}
