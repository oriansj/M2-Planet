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
#include "cc.h"

/* Imported functions */
int strtoint(char *a);
void line_error(void);
void require(int bool, char* error);
struct token_list* sym_lookup(char*, struct token_list*);
int constant_expression(void);

/* enable easy primitive extension */
struct type* add_primitive(struct type* a)
{
	if(NULL == prim_types) return a;
	struct type* i = prim_types;
	while(NULL != i->next)
	{
		i = i->next;
	}
	i->next = a;

	return prim_types;
}

/* enable easy primitive creation */
struct type* new_primitive(char* name0, char* name1, char* name2, int size, int sign)
{
	/* Create type** */
	struct type* a = calloc(1, sizeof(struct type));
	require(NULL != a, "Exhausted memory while declaring new primitive**\n");
	a->name = name2;
	a->size = register_size;
	a->indirect = a;
	a->is_signed = sign;

	/* Create type* */
	struct type* b = calloc(1, sizeof(struct type));
	require(NULL != b, "Exhausted memory while declaring new primitive*\n");
	b->name = name1;
	b->size = register_size;
	b->is_signed = sign;
	b->indirect = a;
	a->type = b;

	struct type* r = calloc(1, sizeof(struct type));
	require(NULL != r, "Exhausted memory while declaring new primitive\n");
	r->name = name0;
	r->size = size;
	r->is_signed = sign;
	r->indirect = b;
	r->type = r;
	b->type = r;

	return r;
}

/* Initialize default types */
void initialize_types(void)
{
	if(AMD64 == Architecture || AARCH64 == Architecture || RISCV64 == Architecture) register_size = 8;
	else register_size = 4;

	/* Define void */
	struct type* hold = new_primitive("void", "void*", "void**", register_size, FALSE);
	prim_types = add_primitive(hold);

	/* Define unsigned LONG */
	hold = new_primitive("SCM","SCM*", "SCM**", register_size, FALSE);
	prim_types = add_primitive(hold);

	/* Define unsigned long long */
	unsigned_long_long = new_primitive("unsigned long long", "unsigned long long*", "unsigned long long**", register_size, FALSE);
	prim_types = add_primitive(unsigned_long_long);

	/* Define signed long long */
	signed_long_long = new_primitive("long long", "long long*", "long long**", register_size, TRUE);
	prim_types = add_primitive(signed_long_long);

	/* Define LONG */
	signed_long = new_primitive("long", "long*", "long**", register_size, TRUE);
	prim_types = add_primitive(signed_long);

	/* Define unsigned long */
	unsigned_long = new_primitive("unsigned long", "unsigned long*", "unsigned long**", register_size, FALSE);
	prim_types = add_primitive(unsigned_long);

	/* Define UNSIGNED */
	unsigned_integer = new_primitive("unsigned", "unsigned*", "unsigned**", register_size, FALSE);
	prim_types = add_primitive(unsigned_integer);

	/* Define int */
	integer = new_primitive("int", "int*", "int**", register_size, TRUE);
	prim_types = add_primitive(integer);

	/* Define signed short */
	signed_short = new_primitive("short", "short*", "short**", 2, TRUE);
	prim_types = add_primitive(signed_short);

	/* Define unsigned short */
	unsigned_short = new_primitive("unsigned short", "unsigned short*", "unsigned short**", 2, FALSE);
	prim_types = add_primitive(unsigned_short);

	/* Define uint64_t */
	hold = new_primitive("uint64_t", "uint64_t*", "uint64_t**", 8, FALSE);
	prim_types = add_primitive(hold);

	/* Define int64_t */
	hold = new_primitive("int64_t", "int64_t*", "int64_t**", 8, TRUE);
	prim_types = add_primitive(hold);

	/* Define uint32_t */
	hold = new_primitive("uint32_t", "uint32_t*", "uint32_t**", 4, FALSE);
	prim_types = add_primitive(hold);

	/* Define int32_t */
	hold = new_primitive("int32_t", "int32_t*", "int32_t**", 4, TRUE);
	prim_types = add_primitive(hold);

	/* Define uint16_t */
	hold = new_primitive("uint16_t", "uint16_t*", "uint16_t**", 2, FALSE);
	prim_types = add_primitive(hold);

	/* Define int16_t */
	hold = new_primitive("int16_t", "int16_t*", "int16_t**", 2, TRUE);
	prim_types = add_primitive(hold);

	/* Define uint8_t */
	hold = new_primitive("uint8_t", "uint8_t*", "uint8_t**", 1, FALSE);
	prim_types = add_primitive(hold);

	/* Define int8_t */
	hold = new_primitive("int8_t", "int8_t*", "int8_t**", 1, TRUE);
	prim_types = add_primitive(hold);

	/* Define char */
	hold = new_primitive("char", "char*", "char**", 1, TRUE);
	prim_types = add_primitive(hold);

	/* Define signed char */
	signed_char = new_primitive("signed char", "signed char*", "signed char**", 1, TRUE);
	prim_types = add_primitive(signed_char);

	/* Define unsigned char */
	unsigned_char = new_primitive("unsigned char", "unsigned char*", "unsigned char**", 1, FALSE);
	prim_types = add_primitive(unsigned_char);

	/* Define _Bool */
	hold = new_primitive("_Bool", "_Bool*", "_Bool**", 1, TRUE);
	prim_types = add_primitive(hold);

	/* Define FUNCTION */
	function_pointer = new_primitive("FUNCTION", "FUNCTION*", "FUNCTION**", register_size, FALSE);
	function_pointer->options = TO_FUNCTION_POINTER; /* FUNCTION */
	function_pointer->indirect->options = TO_FUNCTION_POINTER; /* FUNCTION* */
	prim_types = add_primitive(function_pointer);

	/* Define _va_list */
	hold = new_primitive("__va_list", "__va_list*", "__va_list**", register_size, FALSE);
	prim_types = add_primitive(hold);

	if(BOOTSTRAP_MODE)
	{
		/* Define FILE */
		hold = new_primitive("FILE", "FILE*", "FILE**", register_size, TRUE);
		prim_types = add_primitive(hold);

		/* Primitives mes.c wanted */
		hold = new_primitive("size_t", "size_t*", "size_t**", register_size, FALSE);
		prim_types = add_primitive(hold);

		hold = new_primitive("ssize_t", "ssize_t*", "ssize_t**", register_size, FALSE);
		prim_types = add_primitive(hold);
	}

	global_types = prim_types;
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

struct type* lookup_primitive_type(void)
{
	if(BOOTSTRAP_MODE)
	{
		return lookup_type(global_token->s, prim_types);
	}

	/* Lookup order for multi token types

	 * unsigned
     *	 char
     *	 short
     *	 short int
     *	 long
     *	 long int
     *	 long long
     *	 long long int
     *	 int
     *	 - (unsigned int)

	 * signed
     *	 char
     *	 short
     *	 short int
     *	 long
     *	 long int
     *	 long long
     *	 long long int
     *	 int
	 *	 - (int)

	 * short
     *	 int
     *	 - (short)

	 * long
     *	 int
     *	 long
     *	 long long int
     *	 - (long)
	 */

	if(match("unsigned", global_token->s))
	{
		require(global_token->next != NULL, "NULL token received in multi token type lookup");

		if(match("char", global_token->next->s))
		{
			global_token = global_token->next;
			return unsigned_char;
		}
		else if(match("short", global_token->next->s))
		{
			global_token = global_token->next;
			require(global_token->next != NULL, "NULL token received in multi token type lookup 'unsigned short'");

			if(match("int", global_token->next->s))
			{
				global_token = global_token->next;
				/* fallthrough to unsigned_short */
			}

			return unsigned_short;
		}
		else if(match("long", global_token->next->s))
		{
			global_token = global_token->next;
			require(global_token->next != NULL, "NULL token received in multi token type lookup 'unsigned long'");

			if(match("long", global_token->next->s))
			{
				global_token = global_token->next;
				require(global_token->next != NULL, "NULL token received in multi token type lookup 'unsigned long long'");

				if(match("int", global_token->next->s))
				{
					global_token = global_token->next;
					/* fallthrough to unsigned_long_long */
				}

				return unsigned_long_long;
			}
			else if(match("int", global_token->next->s))
			{
				global_token = global_token->next;
				/* fallthrough to unsigned_long */
			}

			return unsigned_long;
		}
		else if(match("int", global_token->next->s))
		{
			global_token = global_token->next;
			/* fallthrough to unsigned_integer */
		}

		return unsigned_integer;
	}
	else if(match("signed", global_token->s))
	{
		require(global_token->next != NULL, "NULL token received in multi token type lookup");

		if(match("char", global_token->next->s))
		{
			global_token = global_token->next;
			return signed_char;
		}
		else if(match("short", global_token->next->s))
		{
			global_token = global_token->next;
			require(global_token->next != NULL, "NULL token received in multi token type lookup 'signed short'");

			if(match("int", global_token->next->s))
			{
				global_token = global_token->next;
				/* fallthrough to signed_short */
			}

			return signed_short;
		}
		else if(match("long", global_token->next->s))
		{
			global_token = global_token->next;
			require(global_token->next != NULL, "NULL token received in multi token type lookup 'signed long'");

			if(match("long", global_token->next->s))
			{
				global_token = global_token->next;
				require(global_token->next != NULL, "NULL token received in multi token type lookup 'signed long long'");

				if(match("int", global_token->next->s))
				{
					global_token = global_token->next;
					/* fallthrough to signed_long_long */
				}

				return signed_long_long;
			}
			else if(match("int", global_token->next->s))
			{
				global_token = global_token->next;
				/* fallthrough to signed_long */
			}

			return signed_long;
		}
		else if(match("int", global_token->next->s))
		{
			global_token = global_token->next;
			/* fallthrough to integer */
		}

		return integer;
	}
	else if(match("short", global_token->s))
	{
		require(global_token->next != NULL, "NULL token received in multi token type lookup 'short'");

		if(match("int", global_token->next->s))
		{
			global_token = global_token->next;
			/* fallthrough to signed_short */
		}

		return signed_short;
	}
	else if(match("long", global_token->s))
	{
		require(global_token->next != NULL, "NULL token received in multi token type lookup 'long'");

		if(match("long", global_token->next->s))
		{
			global_token = global_token->next;
			require(global_token->next != NULL, "NULL token received in multi token type lookup 'long long'");

			if(match("int", global_token->next->s))
			{
				global_token = global_token->next;
				/* fallthrough to signed_long_long */
			}

			return signed_long_long;
		}
		else if(match("int", global_token->next->s))
		{
			global_token = global_token->next;
			/* fallthrough to signed_long */
		}

		return signed_long;
	}

	return lookup_type(global_token->s, prim_types);
}

struct type* lookup_global_type(void)
{
	struct type* a = lookup_primitive_type();
	if(NULL != a) return a;

	return lookup_type(global_token->s, global_types);
}

struct type* lookup_member(struct type* parent, char* name)
{
	int is_anonymous_type = match("", parent->name);
	if(is_anonymous_type)
	{
		/* We need to be able to know if we're in an anonymous type */
		parent = parent->type;
	}

	struct type* i;
	require(NULL != parent, "Not a valid struct type\n");
	struct type* anonymous;
	for(i = parent->members; NULL != i; i = i->members)
	{
		if(match("", i->name))
		{
			/* Anonymous struct/union (C11 extension */
			 anonymous = lookup_member(i, name);
			 if(anonymous != NULL)
			 {
				 return anonymous;
			 }
		}
		else if(match(i->name, name)) return i;
	}

	/* Anonymous types are not guaranteed to have the member in them */
	if(is_anonymous_type)
	{
		return NULL;
	}

	fputs("ERROR in lookup_member ", stderr);
	fputs(parent->name, stderr);
	fputs("->", stderr);
	fputs(name, stderr);
	fputs(" does not exist\n", stderr);
	line_error();
	fputs("\n", stderr);
	exit(EXIT_FAILURE);
}

struct type* type_name(void);
void require_match(char* message, char* required);

int member_size;
struct type* build_member(struct type* last, int offset)
{
	struct type* i = calloc(1, sizeof(struct type));
	require(NULL != i, "Exhausted memory while building a struct member\n");
	i->members = last;
	i->offset = offset;

	struct type* member_type = type_name();
	require(NULL != member_type, "struct member type can not be invalid\n");
	i->type = member_type;

	if(global_token->s[0] == '(')
	{
		require_extra_token(); /* skip '(' */
		require_match("Required '*' after '(' in struct function pointer.", "*");

		i->name = global_token->s;
		require_extra_token();

		require_match("Required ')' after name in struct function pointer.", ")");
		require_match("Required '(' after ')' in struct function pointer.", "(");

		while(global_token->s[0] != ')')
		{
			type_name();

			if(global_token->s[0] == ',')
			{
				require_extra_token();
			}
		}
		require_extra_token(); /* skip ')' */

		i->type = function_pointer;
	}
	else if(global_token->s[0] != ';')
	{
		i->name = global_token->s;
		require_extra_token();
	}
	else
	{
		struct type* iterator = i->type->members;
		if(iterator == NULL)
		{
			line_error();
			fputs("Missing name for non-struct/union type.\n", stderr);
			exit(EXIT_FAILURE);
		}

		if(!match(i->type->name, "anonymous struct") && !match(i->type->name, "anonymous union"))
		{
			line_error();
			fputs("Anonymous members can not have a type name.\n", stderr);
			exit(EXIT_FAILURE);
		}

		/* Anonymous struct/union (C11 extension) */
		i->name = "";

		/* We need to offset all the member so that they're pointing correctly
		 * into the current struct. */
		while (iterator != NULL)
		{
			iterator->offset = iterator->offset + offset;

			iterator = iterator->members;
		}
	}

	/* Check to see if array */
	if(match( "[", global_token->s))
	{
		require_extra_token();
		i->size = constant_expression() * member_type->type->size;
		if(0 == i->size)
		{
			fputs("Struct only supports [num] form\n", stderr);
			exit(EXIT_FAILURE);
		}
		require_match("Struct only supports [num] form\n", "]");
	}
	else
	{
		i->size = member_type->size;
	}
	member_size = i->size;

	return i;
}

struct type* reverse_members_type_list(struct type* head)
{
	struct type* root = NULL;
	struct type* next;
	while(NULL != head)
	{
		next = head->members;
		head->members = root;
		root = head;
		head = next;
	}
	return root;
}

struct type* create_forward_declared_struct(char* name, int prepend_to_global_types)
{
	struct type* head = calloc(1, sizeof(struct type));
	require(NULL != head, "Exhausted memory while creating a struct\n");
	struct type* i = calloc(1, sizeof(struct type));
	require(NULL != i, "Exhausted memory while creating a struct indirection\n");
	struct type* ii = calloc(1, sizeof(struct type));
	require(NULL != ii, "Exhausted memory while creating a struct double indirection\n");

	head->name = name;
	head->type = head;
	head->indirect = i;
	head->next = global_types;
	head->size = NO_STRUCT_DEFINITION;
	head->members = NULL;

	i->name = head->name;
	i->type = head;
	i->indirect = ii;
	i->size = register_size;
	i->members = NULL;

	ii->name = head->name;
	ii->type = i;
	ii->indirect = ii;
	ii->size = register_size;

	if(prepend_to_global_types)
	{
		global_types = head;
	}

	return head;
}

struct type* create_struct(int is_union)
{
	int offset = 0;
	member_size = 0;

	struct type* head = NULL;
	struct type* i = NULL;

	char* name = "anonymous struct";
	if(is_union)
	{
		name = "anonymous union";
	}

	int has_name = global_token->s[0] != '{';
	if(has_name)
	{
		name = global_token->s;
		head = lookup_global_type();
		require_extra_token();
	}

	if(NULL == head)
	{
		head = create_forward_declared_struct(name, has_name);
		i = head->indirect;
	}
	else
	{
		if(head->size != NO_STRUCT_DEFINITION)
		{
			line_error();
			fputs("struct '", stderr);
			fputs(head->name, stderr);
			fputs("' already has definition.", stderr);
			exit(EXIT_FAILURE);
		}

		i = head->indirect;
	}

	require(NULL != global_token, "Incomplete struct declaration/definition at end of file\n");

	if(global_token->s[0] != '{')
	{
		/*
		 * When forward declaring the struct will have size == 0 and be an error to use.
		 * Zero-sized types are not allowed in C so this will never happen naturally.
		 */
		return head;
	}

	int largest_member_size = 0;
	require_match("ERROR in create_struct\n Missing {\n", "{");
	struct type* last = NULL;
	require(NULL != global_token, "Incomplete struct definition at end of file\n");
	while('}' != global_token->s[0])
	{
		last = build_member(last, offset);

		if(member_size == NO_STRUCT_DEFINITION)
		{
			line_error();
			fputs("Can not use non-defined type in object.\n", stderr);
			exit(EXIT_FAILURE);
		}

		offset = offset + member_size;
		if(member_size > largest_member_size)
		{
			largest_member_size = member_size;
		}

		if(is_union)
		{
			offset = 0;
		}

		require_match("ERROR in create_struct\n Missing ;\n", ";");
		require(NULL != global_token, "Unterminated struct\n");
	}

	/* Members are prepended so the list needs to be reversed. */
	last = reverse_members_type_list(last);

	require_extra_token();

	head->size = offset;
	if(is_union)
	{
		head->size = largest_member_size;
	}

	head->members = last;
	i->members = last;

	return head;
}

struct type* create_enum(void)
{
	maybe_bootstrap_error("enum statement");
	struct type* head = calloc(1, sizeof(struct type));
	require(NULL != head, "Exhausted memory while creating an enum\n");
	struct type* i = calloc(1, sizeof(struct type));
	require(NULL != i, "Exhausted memory while creating a enum indirection\n");
	struct type* ii = calloc(1, sizeof(struct type));
	require(NULL != ii, "Exhausted memory while creating a enum double indirection\n");

	head->type = head;
	head->indirect = i;
	head->next = global_types;

	head->size = register_size; /* We treat enums as always being ints. */
	head->is_signed = TRUE;

	i->name = head->name;
	i->type = head;
	i->indirect = ii;
	i->size = register_size;

	ii->name = head->name;
	ii->type = i;
	ii->indirect = ii;
	ii->size = register_size;

	if(match("{", global_token->s))
	{
		head->name = "anonymous enum";
	}
	else
	{
		head->name = global_token->s;
		require_extra_token();

		/* Anonymous enums should not be able to be looked up
		 * so we only add named enums. */
		global_types = head;
	}

	require_match("ERROR in create_enum\n Missing {\n", "{");
	require(NULL != global_token, "Incomplete enum definition at end of file\n");

	int next_enum_value = 0;
	int expr = 0;
	while('}' != global_token->s[0])
	{
		global_constant_list = sym_declare(global_token->s, NULL, global_constant_list, TLO_CONSTANT);
		global_constant_list->type = integer;

		require_extra_token();

		global_constant_list->arguments = calloc(1, sizeof(struct token_list));
		if(match("=", global_token->s))
		{
			require_extra_token();

			expr = constant_expression();
		}

		global_constant_list->arguments->s = int2str(expr, 10, TRUE);
		next_enum_value = expr + 1;
		expr = next_enum_value;

		if(match(",", global_token->s))
		{
			require_extra_token();
		}

		require(NULL != global_token, "Unterminated enum\n");
	}

	require_extra_token();

	return head;
}

struct type* fallible_type_name(void)
{
	struct type* ret;

	require(NULL != global_token, "Received EOF instead of type name\n");

	if(match("extern", global_token->s))
	{
		require_extra_token();
	}

	if(match("const", global_token->s))
	{
		require_extra_token();
	}

	if(match("struct", global_token->s))
	{
		require_extra_token();
		ret = lookup_global_type();
		if(match(global_token->s, "{") || match(global_token->next->s, "{") || match(global_token->next->s, ";"))
		{
			return create_struct(FALSE);
		}
		else if(NULL == ret)
		{
			ret = create_forward_declared_struct(global_token->s, TRUE);
		}
	}
	else if(match("enum", global_token->s))
	{
		maybe_bootstrap_error("enum statements");
		require_extra_token();
		ret = lookup_global_type();
		if(NULL == ret)
		{
			return create_enum();
		}
	}
	else if(match("union", global_token->s))
	{
		require_extra_token();
		ret = lookup_global_type();
		if(match(global_token->s, "{") || match(global_token->next->s, "{") || match(global_token->next->s, ";"))
		{
			return create_struct(TRUE);
		}
		else if(NULL == ret)
		{
			ret = create_forward_declared_struct(global_token->s, TRUE);
		}
	}
	else
	{
		ret = lookup_global_type();
		if(NULL == ret)
		{
			return NULL;
		}
	}

	require_extra_token();

	if(match("const", global_token->s))
	{
		require_extra_token();
	}

	while(global_token->s[0] == '*')
	{
		ret = ret->indirect;
		require_extra_token();

		while(match("const", global_token->s) || match("restrict", global_token->s))
		{
			require_extra_token();
		}
	}

	return ret;
}

struct type* type_name(void)
{
	struct type* ret = fallible_type_name();
	if(ret != NULL)
	{
		return ret;
	}

	fputs("Unknown type ", stderr);
	fputs(global_token->s, stderr);
	fputs("\n", stderr);
	line_error();
	fputs("\n", stderr);
	exit(EXIT_FAILURE);
}

struct type* new_function_pointer_typedef(char* name)
{
	struct type* first = new_primitive(name, name, name, register_size, FALSE);
	first->options = TO_FUNCTION_POINTER;
	first->indirect->options = TO_FUNCTION_POINTER;

	return add_primitive(first);
}

struct type *mirror_type(struct type *source)
{
	struct type* head = lookup_primitive_type();
	struct type* i;
	if(NULL == head)
	{
		head = calloc(1, sizeof(struct type));
		require(NULL != head, "Exhausted memory while creating a struct\n");

		add_primitive(head);

		i = calloc(1, sizeof(struct type));
		require(NULL != i, "Exhausted memory while creating a struct indirection\n");
	}
	else
	{
		i = head->indirect;
	}

	head->name = global_token->s;
	i->name = global_token->s;
	head->size = source->size;
	i->size = source->indirect->size;
	head->offset = source->offset;
	i->offset = source->indirect->offset;
	head->is_signed = source->is_signed;
	i->is_signed = source->indirect->is_signed;
	head->indirect = i;
	i->indirect = head;
	head->members = source->members;
	i->members =  source->indirect->members;
	head->type = head;
	i->type = i;

	return head;
}
