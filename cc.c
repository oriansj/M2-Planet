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


#define GLOBAL 1
#define FUNCTION 2
#define LOCAL_VARIABLE 4
#define ARGUEMENT 8

/* Globals */
struct token_list* global_symbol_list;
struct token_list* global_stack;
struct token_list* global_token;

/* Imported functions */
int asprintf(char **strp, const char *fmt, ...);
struct token_list* read_all_tokens(char* source_file);

struct token_list* emit(char *s, bool hands_off, struct token_list* head)
{
	struct token_list* t = calloc(1, sizeof(struct token_list));
	t->next = head;
	t->hands_off = hands_off;
	t->s = s;
	return t;
}

struct token_list* pull_value_off_stack(int register_number, struct token_list* out)
{
	global_stack = global_stack->next;

	if(0 == register_number) out = emit("POP_eax\n", true, out);
	else if(1 == register_number) out = emit("POP_ebx\n", true, out);
	else exit(EXIT_FAILURE);
	return out;
}

void add_to_frame(struct token_list* a)
{
	struct token_list* new = calloc(1, sizeof(struct token_list));
	new->entry = a;
	new->next = global_stack;
	global_stack = new;
}

struct token_list* put_value_on_stack(int register_number, struct token_list* a, struct token_list* out)
{
	add_to_frame(a);

	if(0 == register_number) out = emit("PUSH_eax\n", true, out);
	else if(1 == register_number) out = emit("PUSH_ebx\n", true, out);
	else exit(EXIT_FAILURE);
	return out;
}

struct token_list* be_pop_type(struct token_list* floor, int type, struct token_list* out)
{
	bool flag = false;
	for( struct token_list* i = global_stack; ((!flag) && (floor != i) && ((NULL == i->entry) || (i->entry->type == type))); i = global_stack)
	{
		if((NULL != i->entry) && (i->entry->type == FUNCTION)) flag = true;
		out = pull_value_off_stack(1, out);
	}
	return out;
}

struct token_list* sym_declare(char *s, int type)
{
	struct token_list* a = calloc(1, sizeof(struct token_list));
	a->next = global_symbol_list;
	global_symbol_list = a;

	a->type = type;
	a->s = s;
	return a;
}

struct token_list* sym_lookup(char *s, struct token_list* symbol_list)
{
	for(struct token_list* i = symbol_list; NULL != i; i = i->next)
	{
		if(0 == strcmp(s,i->s)) return i;
	}
	return NULL;
}

int stack_index(struct token_list* a)
{
	int index = 0;
	for(struct token_list* b = global_stack; NULL != b; (b = b->next) && (index = index + 1))
	{
		if(b->entry == a) return index;
	}
	return index;
}

struct token_list* sym_get_value(char *s, struct token_list* out)
{
	char* label;
	struct token_list* a = sym_lookup(s, global_symbol_list);
	if(a == NULL) exit(EXIT_FAILURE);

	switch(a->type)
	{
		case GLOBAL: asprintf(&label, "LOAD_IMMEDIATE_eax &GLOBAL_%s\n", s); break;
		case LOCAL_VARIABLE: asprintf(&label, "LOAD_EFFECTIVE_ADDRESS %c%d\n", 37, 4 * stack_index(a)); break;
		case ARGUEMENT: asprintf(&label, "LOAD_EFFECTIVE_ADDRESS %c%d\n", 37, 4 * (stack_index(a) + 1)); break;
		case FUNCTION:
		{
			asprintf(&label, "LOAD_IMMEDIATE_eax &FUNCTION_%s\n", s);
			out = emit(label, true, out);
			out = put_value_on_stack(0, a, out);
			return out;
		}
		default: exit(EXIT_FAILURE);
	}
	out = emit(label, true, out);
	if(strcmp(global_token->next->s, "=")) out = emit("LOAD_INTEGER\n", true, out);
	return out;
}

void require_char(char* message, char required)
{
	if(global_token->s[0] != required)
	{
		fprintf(stderr, "%s", message);
		exit(EXIT_FAILURE);
	}
	global_token = global_token->next;
}

struct token_list* expression(struct token_list* out);
struct token_list* parse_string(struct token_list* output_list, char* string);

/*
 * primary-expr:
 *     identifier
 *     constant
 *     ( expression )
 */
struct token_list* primary_expr(struct token_list* out)
{
	char* label;

	if(('0' <= global_token->s[0]) & (global_token->s[0] <= '9'))
	{
		asprintf(&label, "LOAD_IMMEDIATE_eax %c%s\n", 37, global_token->s);
		out = emit(label, true, out);
		global_token = global_token->next;
	}
	else if(('a' <= global_token->s[0]) & (global_token->s[0] <= 'z'))
	{
		out = sym_get_value(global_token->s, out);
		global_token = global_token->next;
	}
	else if(global_token->s[0] == '(')
	{
		global_token = global_token->next;
		out = expression(out);
		require_char("Error in Primary expression\nDidn't get )\n", ')');
	}
	else if((global_token->s[0] == 39) & (global_token->s[1] != 0) & (global_token->s[2] == 39) & (global_token->s[3] == 0))
	{
		asprintf(&label, "LOAD_IMMEDIATE_eax %c%d\n", 37, global_token->s[1]);
		out = emit(label, true, out);
		global_token = global_token->next;
	}
	else if(global_token->s[0] == '"')
	{
		out = parse_string(out, global_token->s);
		global_token = global_token->next;
	}
	else exit(EXIT_FAILURE);

	return out;
}

/* Deal with Expression lists */
struct token_list* process_expression_list(struct token_list* out)
{
	global_token = global_token->next;
	struct token_list* function = global_stack->entry;

	if(global_token->s[0] != ')')
	{
		out = expression(out);
		out = put_value_on_stack(0, NULL, out);

		while(global_token->s[0] == ',')
		{
			global_token = global_token->next;
			out = expression(out);
			out = put_value_on_stack(0, NULL, out);
		}
		require_char("ERROR in process_expression_list\nNo ) was found\n", ')');
	}
	else global_token = global_token->next;

	char* label;
	asprintf(&label, "LOAD_ESP_IMMEDIATE_into_eax %c%d\nCALL_eax\n", 37, 4 * stack_index(function));
	out = emit(label, true, out);
	out = be_pop_type(NULL, FUNCTION, out);
	return out;
}

struct token_list* common_recursion(struct token_list* (*function) (struct token_list*), struct token_list* out)
{
	global_token = global_token->next;
	out = put_value_on_stack(0, NULL, out);
	out = function(out);
	out = pull_value_off_stack(1, out);
	return out;
}

/*
 * postfix-expr:
 *         primary-expr
 *         postfix-expr [ expression ]
 *         postfix-expr ( expression-list-opt )
 */
struct token_list* postfix_expr(struct token_list* out)
{
	out = primary_expr(out);

	if(global_token->s[0] == '[')
	{
		out = common_recursion(expression, out);
		out = emit("ADD_ebx_to_eax\n", true, out);
		if(strcmp(global_token->next->s, "=")) out = emit("LOAD_BYTE\n", true, out);
		require_char("ERROR in postfix_expr\nMissing ]\n", ']');
	}
	else if(global_token->s[0] == '(') out = process_expression_list(out);

	return out;
}

/*
 * additive-expr:
 *         postfix-expr
 *         additive-expr + postfix-expr
 *         additive-expr - postfix-expr
 */
struct token_list* additive_expr(struct token_list* out)
{
	out = postfix_expr(out);

	while(1)
	{
		if(global_token->s[0] == '+')
		{
			out = common_recursion(postfix_expr, out);
			out = emit("ADD_ebx_to_eax\n", true, out);
		}
		else if(global_token->s[0] == '-')
		{
			out = common_recursion(postfix_expr, out);
			out = emit("SUBTRACT_eax_from_ebx_into_ebx\nMOVE_ebx_to_eax\n", true, out);
		}
		else return out;
	}
}

/*
 * shift-expr:
 *         additive-expr
 *         shift-expr << additive-expr
 *         shift-expr >> additive-expr
 */
struct token_list* shift_expr(struct token_list* out)
{
	out = additive_expr(out);

	while(1)
	{
		if(!strcmp(global_token->s, "<<"))
		{
			out = common_recursion(additive_expr, out);
			// Ugly hack to Work around flaw in x86
			out = emit("COPY_eax_to_ecx\nPOP_eax\nSAL_eax_cl\n", true, out->next);
		}
		else if(!strcmp(global_token->s, ">>"))
		{
			out = common_recursion(additive_expr, out);
			// Ugly hack to Work around flaw in x86
			out = emit("COPY_eax_to_ecx\nPOP_eax\nSAR_eax_cl\n", true, out->next);
		}
		else
		{
			return out;
		}
	}
}

/*
 * relational-expr:
 *         shift-expr
 *         relational-expr <= shift-expr
 */
struct token_list* relational_expr(struct token_list* out)
{
	out = shift_expr(out);

	while(!strcmp(global_token->s, "<="))
	{
		out = common_recursion(shift_expr, out);
		out = emit("CMP\nSETLE\nMOVEZBL\n", true, out);
	}
	return out;
}

/*
 * equality-expr:
 *         relational-expr
 *         equality-expr == relational-expr
 *         equality-expr != relational-expr
 */
struct token_list* equality_expr(struct token_list* out)
{
	out = relational_expr(out);

	while(1)
	{
		if(!strcmp(global_token->s, "=="))
		{
			out = common_recursion(relational_expr, out);
			out = emit("CMP\nSETE\nMOVEZBL\n", true, out);
		}
		else if(!strcmp(global_token->s, "!="))
		{
			out = common_recursion(relational_expr, out);
			out = emit("CMP\nSETNE\nMOVEZBL\n", true, out);
		}
		else return out;
	}
}

/*
 * bitwise-and-expr:
 *         equality-expr
 *         bitwise-and-expr & equality-expr
 */
struct token_list* bitwise_and_expr(struct token_list* out)
{
	out = equality_expr(out);

	while(global_token->s[0] == '&')
	{
		out = common_recursion(equality_expr, out);
		out = emit("AND_eax_ebx\n", true, out);
	}
	return out;
}

/*
 * bitwise-or-expr:
 *         bitwise-and-expr
 *         bitwise-and-expr | bitwise-or-expr
 */
struct token_list* bitwise_or_expr(struct token_list* out)
{
	out = bitwise_and_expr(out);

	while(global_token->s[0] == '|')
	{
		out = common_recursion(bitwise_and_expr, out);
		out = emit("OR_eax_ebx\n", true, out);
	}
	return out;
}

/*
 * expression:
 *         bitwise-or-expr
 *         bitwise-or-expr = expression
 */
struct token_list* expression(struct token_list* out)
{
	out = bitwise_or_expr(out);

	if(global_token->s[0] == '=')
	{
		bool byte = !strcmp(global_token->prev->s, "]");
		out = common_recursion(expression, out);
		if(!byte) out = emit("STORE_INTEGER\n", true, out);
		else out = emit("STORE_CHAR\n", true, out);
	}
	return out;
}

/*
 * type-name:
 *     char *
 *     int
 */
void type_name()
{
	global_token = global_token->next;

	while(global_token->s[0] == '*')
	{
		global_token = global_token->next;
	}
}

struct token_list* statement(struct token_list* out);

/* Process local variable */
struct token_list* collect_local(struct token_list* out)
{
	type_name();
	char* label;
	asprintf(&label, "# Defining local %s\n", global_token->s);
	out = emit(label, true, out);

	struct token_list* a = sym_declare(global_token->s, LOCAL_VARIABLE);
	global_token = global_token->next;

	if(global_token->s[0] == '=')
	{
		global_token = global_token->next;
		out = expression(out);
	}

	require_char("ERROR in collect_local\nMissing ;\n", ';');
	out = put_value_on_stack(0, a, out);
	return out;
}

/* Evaluate if statements */
int if_count;
struct token_list* process_if(struct token_list* out)
{
	char* label;
	int number = if_count;
	if_count = if_count + 1;

	asprintf(&label, "# IF_%d\n", number);
	out = emit(label, true, out);

	global_token = global_token->next;
	require_char("ERROR in process_if\nMISSING (\n", '(');
	out = expression(out);

	asprintf(&label, "TEST\nJUMP_EQ %c%s_%d\n", 37, "ELSE",  number);
	out = emit(label, true, out);

	require_char("ERROR in process_if\nMISSING )\n", ')');
	out = statement(out);

	asprintf(&label, "JUMP %c_END_IF_%d\n:ELSE_%d\n", 37, number, number);
	out = emit(label, true, out);

	if(!strcmp(global_token->s, "else"))
	{
		global_token = global_token->next;
		out = statement(out);
	}
	asprintf(&label, ":_END_IF_%d\n", number);
	out = emit(label, true, out);
	return out;
}

int for_count;
struct token_list* process_for(struct token_list* out)
{
	char* label;
	int number = for_count;
	for_count = for_count + 1;

	asprintf(&label, "# FOR_initialization_%d\n", number);
	out = emit(label, true, out);

	global_token = global_token->next;

	require_char("ERROR in process_for\nMISSING (\n", '(');
	out = expression(out);

	asprintf(&label, ":FOR_%d\n", number);
	out = emit(label, true, out);

	require_char("ERROR in process_for\nMISSING ;1\n", ';');
	out = expression(out);

	asprintf(&label, "TEST\nJUMP_EQ %c%s_%d\nJUMP %c%s_%d\n:FOR_ITER_%d\n", 37, "FOR_END", number, 37, "FOR_THEN", number, number);
	out = emit(label, true, out);

	require_char("ERROR in process_for\nMISSING ;2\n", ';');
	out = expression(out);

	asprintf(&label, "JUMP %c%s_%d\n:FOR_THEN_%d\n", 37, "FOR", number, number);
	out = emit(label, true, out);

	require_char("ERROR in process_for\nMISSING )\n", ')');
	out = statement(out);

	asprintf(&label, "JUMP %c%s_ITER_%d\n:FOR_END_%d\n", 37, "FOR", number, number);
	out = emit(label, true, out);
	return out;
}

/* Process while loops */
int while_count;
struct token_list* process_while(struct token_list* out)
{
	char* label;
	int number = while_count;
	while_count = while_count + 1;

	asprintf(&label, ":WHILE_%d\n", number);
	out = emit(label, true, out);

	global_token = global_token->next;

	require_char("ERROR in process_while\nMISSING (\n", '(');
	out = expression(out);

	asprintf(&label, "TEST\nJUMP_EQ %c%s_WHILE_%d\n# THEN_while_%d\n", 37, "END", number, number);
	out = emit(label, true, out);

	require_char("ERROR in process_while\nMISSING )\n", ')');
	out = statement(out);

	asprintf(&label, "JUMP %c%s_%d\n:END_WHILE_%d\n", 37, "WHILE", number, number);
	out = emit(label, true, out);
	return out;
}

/* Ensure that functions return */
struct token_list* return_result(struct token_list* out)
{
	global_token = global_token->next;
	if(global_token->s[0] != ';') out = expression(out);

	require_char("ERROR in return_result\nMISSING ;\n", ';');
	out = be_pop_type(NULL, LOCAL_VARIABLE, out);
	out = emit("RETURN\n", true, out);
	return out;
}

struct token_list* recursive_statement(struct token_list* out)
{
	global_token = global_token->next;
	struct token_list* frame = global_stack;

	while(strcmp(global_token->s, "}"))
	{
		out = statement(out);
	}
	global_token = global_token->next;

	out = be_pop_type(frame, LOCAL_VARIABLE, out);
	return out;
}

/*
 * statement:
 *     { statement-list-opt }
 *     type-name identifier ;
 *     type-name identifier = expression;
 *     if ( expression ) statement
 *     if ( expression ) statement else statement
 *     while ( expression ) statement
 *     for ( expression ; expression ; expression ) statement
 *     return ;
 *     expr ;
 */
struct token_list* statement(struct token_list* out)
{
	if(global_token->s[0] == '{') out = recursive_statement(out);
	else if((!strcmp(global_token->s, "char")) | (!strcmp(global_token->s, "int"))) out = collect_local(out);
	else if(!strcmp(global_token->s, "if")) out = process_if(out);
	else if(!strcmp(global_token->s, "while")) out = process_while(out);
	else if(!strcmp(global_token->s, "for")) out = process_for(out);
	else if(!strcmp(global_token->s, "return")) out = return_result(out);
	else
	{
		out = expression(out);
		require_char("ERROR in statement\nMISSING ;\n", ';');
	}
	return out;
}

/* Collect function arguments */
struct token_list* collect_arguments(struct token_list* out)
{
	global_token = global_token->next;

	while(strcmp(global_token->s, ")"))
	{
		type_name();

		if(global_token->s[0] != ')')
		{
			char* label;
			asprintf(&label, "# Defining Argument %s\n", global_token->s);
			out = emit(label, true, out);

			struct token_list* a = sym_declare(global_token->s, ARGUEMENT);
			add_to_frame(a);
			global_token = global_token->next;
		}

		/* ignore trailing comma (needed for foo(bar(), 1); expressions*/
		if(global_token->s[0] == ',') global_token = global_token->next;
	}
	global_token = global_token->next;
	return out;
}

struct token_list* declare_global(struct token_list* out)
{
	char* label;
	asprintf(&label, "# Defining global %s\n:GLOBAL_%s\n", global_token->prev->s, global_token->prev->s);
	out = emit(label, true, out);
	sym_declare(global_token->prev->s, GLOBAL);

	global_token = global_token->next;
	return emit("NOP\n", true, out);
}

struct token_list* declare_function(struct token_list* out)
{
	char* label;
	asprintf(&label, "# Defining function %s\n:FUNCTION_%s\n", global_token->prev->s, global_token->prev->s);
	sym_declare(global_token->prev->s, FUNCTION);

	struct token_list* current = global_symbol_list;
	out = collect_arguments(out);

	if(global_token->s[0] != ';')
	{
		out = emit(label, true, out);
		out = statement(out);
		out = emit("RETURN\n", true, out);
	}
	else
	{
		global_token = global_token->next;
	}

	for( struct token_list* i = global_symbol_list; current != i; i = i->next)
	{
		global_symbol_list = i;
	}
	return out;
}

/*
 * program:
 *     declaration
 *     declaration program
 *
 * declaration:
 *     type-name identifier ;
 *     type-name identifier ( parameter-list ) ;
 *     type-name identifier ( parameter-list ) statement
 *
 * parameter-list:
 *     parameter-declaration
 *     parameter-list, parameter-declaration
 *
 * parameter-declaration:
 *     type-name identifier-opt
 */
struct token_list* program(struct token_list* out)
{
	while(NULL != global_token->next)
	{
		type_name();
		global_token = global_token->next;
		if(global_token->s[0] == ';') out = declare_global(out);
		else if(global_token->s[0] == '(') out = declare_function(out);
		else exit(EXIT_FAILURE);
	}
	return out;
}

void recursive_output(FILE* out, struct token_list* i)
{
	if(NULL == i) return;

	recursive_output(out, i->next);

	if(i->hands_off) fprintf(out, "%s", i->s);
	else
	{
		fprintf(out, "'");
		for(int j = 0; 0 != i->s[j]; j = j + 1)
		{
			fputc(i->s[j], out);
		}
		fprintf(out, "'\t# NONE\n");
	}
}

/* Our essential organizer */
int main(int argc, char **argv)
{
	if (argc < 3)
	{
		fprintf(stderr, "We require more arguments\n");
		exit(EXIT_FAILURE);
	}

	global_token = read_all_tokens(argv[1]);
	struct token_list* output_list = program(NULL);
	FILE* output = fopen(argv[2], "w");
	recursive_output(output, output_list);
	fclose(output);
	return 0;
}
