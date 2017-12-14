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
struct token_list* global_token;
struct token_list* current_target;
struct token_list* strings;

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

struct token_list* sym_declare(char *s, int type, struct type* size)
{
	struct token_list* a = calloc(1, sizeof(struct token_list));
	a->next = global_symbol_list;
	global_symbol_list = a;

	a->type = type;
	a->s = s;
	a->size = size;
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

int stack_index(struct token_list* a, struct token_list* function)
{
	int depth = function->temps;
	for(struct token_list* i = function->locals; NULL != i; i = i->locals)
	{
		if(i == a) return depth;
		else depth = depth + 1;
	}

	/* Deal with offset caused by return pointer */
	depth = depth+ 1;

	for(struct token_list* i = function->arguments; NULL != i; i = i->arguments)
	{
		if(i == a) return depth;
		else depth = depth + 1;
	}

	fprintf(stderr, "%s does not exist in function %s\n", a->s, function->s);
	exit(EXIT_FAILURE);
}

struct token_list* sym_get_value(char *s, struct token_list* out, struct token_list* function)
{
	char* label;
	struct token_list* a = sym_lookup(s, global_symbol_list);
	if(a == NULL)
	{
		fprintf(stderr, "%s is not a defined symbol\n", s);
		exit(EXIT_FAILURE);
	}

	current_target = a;

	switch(a->type)
	{
		case GLOBAL: asprintf(&label, "LOAD_IMMEDIATE_eax &GLOBAL_%s\n", s); break;
		case LOCAL_VARIABLE: asprintf(&label, "LOAD_EFFECTIVE_ADDRESS %c%d\n", 37, 4 * stack_index(a, function)); break;
		case ARGUEMENT: asprintf(&label, "LOAD_EFFECTIVE_ADDRESS %c%d\n", 37, 4 * (stack_index(a, function))); break;
		case FUNCTION: return out;
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

struct token_list* expression(struct token_list* out, struct token_list* function);
struct token_list* parse_string(struct token_list* output_list, char* string);

/*
 * primary-expr:
 *     identifier
 *     constant
 *     ( expression )
 */
struct token_list* primary_expr(struct token_list* out, struct token_list* function)
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
		out = sym_get_value(global_token->s, out, function);
		global_token = global_token->next;
	}
	else if(global_token->s[0] == '(')
	{
		global_token = global_token->next;
		out = expression(out, function);
		require_char("Error in Primary expression\nDidn't get )\n", ')');
	}
	else if(global_token->s[0] == '\'')
	{
		asprintf(&label, "LOAD_IMMEDIATE_eax %c%d\n", 37, global_token->s[1]);
		out = emit(label, true, out);
		global_token = global_token->next;
	}
	else if(global_token->s[0] == '"')
	{
		static int string_num;
		asprintf(&label, "LOAD_IMMEDIATE_eax &STRING_%d\n", string_num);
		out = emit(label, true, out);

		/* The target */
		asprintf(&label, ":STRING_%d\n", string_num);
		strings = emit(label, true, strings);

		/* Parse the string */
		strings = parse_string(strings, global_token->s);
		global_token = global_token->next;

		string_num = string_num + 1;
	}
	else exit(EXIT_FAILURE);

	return out;
}

/* Deal with Expression lists */
struct token_list* process_expression_list(struct token_list* out, struct token_list* function)
{
	struct token_list* func = sym_lookup(global_token->prev->s, global_symbol_list);
	global_token = global_token->next;
	int temp = function->temps;

	if(global_token->s[0] != ')')
	{
		char* label;
		out = expression(out, function);
		asprintf(&label, "PUSH_eax\t#%s\n", "_process_expression1");
		out = emit(label, true, out);
		function->temps = function->temps + 1;

		while(global_token->s[0] == ',')
		{
			global_token = global_token->next;
			out = expression(out, function);
			asprintf(&label, "PUSH_eax\t#%s\n", "_process_expression2");
			out = emit(label, true, out);
			function->temps = function->temps + 1;
		}
		require_char("ERROR in process_expression_list\nNo ) was found\n", ')');
	}
	else global_token = global_token->next;
	function->temps = temp;

	char* label;
	asprintf(&label, "CALL_IMMEDIATE %cFUNCTION_%s\n", 37, func->s);
	out = emit(label, true, out);

	for(struct token_list* i = func->arguments; NULL != i; i = i->arguments)
	{
		asprintf(&label, "POP_ebx\t# %s\n", "_process_expression_locals");
		out = emit(label, true, out);
	}

	return out;
}

struct token_list* common_recursion(struct token_list* (*function) (struct token_list*, struct token_list*), struct token_list* out, struct token_list* func)
{
	char* label;
	global_token = global_token->next;
	asprintf(&label, "PUSH_eax\t#%s\n", "_common_recursion");
	out = emit(label, true, out);
	func->temps = func->temps + 1;
	out = function(out, func);
	func->temps = func->temps - 1;
	asprintf(&label, "POP_ebx\t# %s\n", "_common_recursion");
	out = emit(label, true, out);
	return out;
}

/*
 * postfix-expr:
 *         primary-expr
 *         postfix-expr [ expression ]
 *         postfix-expr ( expression-list-opt )
 */
struct token_list* postfix_expr(struct token_list* out, struct token_list* function)
{
	out = primary_expr(out, function);

	if(global_token->s[0] == '[')
	{
		struct token_list* target = current_target;
		out = common_recursion(expression, out, function);

		/* Add support for Ints*/
		if( 4 == target->size->member_size)
		{
			out = emit("SAL_eax_Immediate8 !2\n", true, out);
		}

		out = emit("ADD_ebx_to_eax\n", true, out);
		current_target = target;

		if(strcmp(global_token->next->s, "="))
		{
			if( 4 == target->size->member_size)
			{
				out = emit("LOAD_INTEGER\n", true, out);
			}
			else
			{
				out = emit("LOAD_BYTE\n", true, out);
			}
		}
		require_char("ERROR in postfix_expr\nMissing ]\n", ']');
	}
	else if(global_token->s[0] == '(')
	{
		out = process_expression_list(out, function);
	}

	return out;
}

/*
 * additive-expr:
 *         postfix-expr
 *         additive-expr + postfix-expr
 *         additive-expr - postfix-expr
 */
struct token_list* additive_expr(struct token_list* out, struct token_list* function)
{
	out = postfix_expr(out, function);

	while(1)
	{
		if(global_token->s[0] == '+')
		{
			out = common_recursion(postfix_expr, out, function);
			out = emit("ADD_ebx_to_eax\n", true, out);
		}
		else if(global_token->s[0] == '-')
		{
			out = common_recursion(postfix_expr, out, function);
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
struct token_list* shift_expr(struct token_list* out, struct token_list* function)
{
	out = additive_expr(out, function);

	while(1)
	{
		if(!strcmp(global_token->s, "<<"))
		{
			out = common_recursion(additive_expr, out, function);
			// Ugly hack to Work around flaw in x86
			struct token_list* old = out->next;
			free(out);
			out = emit("COPY_eax_to_ecx\nPOP_eax\nSAL_eax_cl\n", true, old);
		}
		else if(!strcmp(global_token->s, ">>"))
		{
			out = common_recursion(additive_expr, out, function);
			// Ugly hack to Work around flaw in x86
			struct token_list* old = out->next;
			free(out);
			out = emit("COPY_eax_to_ecx\nPOP_eax\nSAR_eax_cl\n", true, old);
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
struct token_list* relational_expr(struct token_list* out, struct token_list* function)
{
	out = shift_expr(out, function);

	while(!strcmp(global_token->s, "<="))
	{
		out = common_recursion(shift_expr, out, function);
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
struct token_list* equality_expr(struct token_list* out, struct token_list* function)
{
	out = relational_expr(out, function);

	while(1)
	{
		if(!strcmp(global_token->s, "=="))
		{
			out = common_recursion(relational_expr, out, function);
			out = emit("CMP\nSETE\nMOVEZBL\n", true, out);
		}
		else if(!strcmp(global_token->s, "!="))
		{
			out = common_recursion(relational_expr, out, function);
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
struct token_list* bitwise_and_expr(struct token_list* out, struct token_list* function)
{
	out = equality_expr(out, function);

	while(global_token->s[0] == '&')
	{
		out = common_recursion(equality_expr, out, function);
		out = emit("AND_eax_ebx\n", true, out);
	}
	return out;
}

/*
 * bitwise-or-expr:
 *         bitwise-and-expr
 *         bitwise-and-expr | bitwise-or-expr
 */
struct token_list* bitwise_or_expr(struct token_list* out, struct token_list* function)
{
	out = bitwise_and_expr(out, function);

	while(global_token->s[0] == '|')
	{
		out = common_recursion(bitwise_and_expr, out, function);
		out = emit("OR_eax_ebx\n", true, out);
	}
	return out;
}

/*
 * expression:
 *         bitwise-or-expr
 *         bitwise-or-expr = expression
 */
struct token_list* expression(struct token_list* out, struct token_list* function)
{
	out = bitwise_or_expr(out, function);

	if(global_token->s[0] == '=')
	{
		struct token_list* target = current_target;
		bool member = !strcmp(global_token->prev->s, "]");
		out = common_recursion(expression, out, function);

		if(member)
		{
			if(1 == target->size->member_size) out = emit("STORE_CHAR\n", true, out);
			else if(4 == target->size->member_size)
			{
				out = emit("STORE_INTEGER\n", true, out);
			}
		}
		else
		{
			out = emit("STORE_INTEGER\n", true, out);
		}
	}
	return out;
}

/*
 * type-name:
 *     char *
 *     int
 */
struct type* type_name()
{
	struct type* ret = calloc(1, sizeof(struct type));
	ret->size = 4;
	if(!strcmp(global_token->s, "char"))
	{
		ret->size = 1;
	}

	if(!strcmp(global_token->s, "void"))
	{
		ret->_void = 1;
	}
	global_token = global_token->next;

	if(global_token->s[0] == '*')
	{
		ret->member_size = ret->size;
		ret->size = 4;
		ret->indirect = 1;
	}

	while(global_token->s[0] == '*')
	{
		global_token = global_token->next;
	}

	return ret;
}

/* Process local variable */
struct token_list* collect_local(struct token_list* out, struct token_list* function)
{
	struct type* type_size = type_name();
	char* label;
	asprintf(&label, "# Defining local %s\n", global_token->s);
	out = emit(label, true, out);

	struct token_list* a = sym_declare(global_token->s, LOCAL_VARIABLE, type_size);
	a->locals = function->locals;
	function->locals = a;
	global_token = global_token->next;
	function->temps = function->temps - 1;

	if(global_token->s[0] == '=')
	{
		global_token = global_token->next;
		out = expression(out, function);
	}
	function->temps = function->temps + 1;

	require_char("ERROR in collect_local\nMissing ;\n", ';');
	asprintf(&label, "PUSH_eax\t#%s\n", a->s);
	out = emit(label, true, out);
	return out;
}

struct token_list* statement(struct token_list* out, struct token_list* function);

/* Evaluate if statements */
int if_count;
struct token_list* process_if(struct token_list* out, struct token_list* function)
{
	char* label;
	int number = if_count;
	if_count = if_count + 1;

	asprintf(&label, "# IF_%d\n", number);
	out = emit(label, true, out);

	global_token = global_token->next;
	require_char("ERROR in process_if\nMISSING (\n", '(');
	out = expression(out, function);

	asprintf(&label, "TEST\nJUMP_EQ %c%s_%d\n", 37, "ELSE",  number);
	out = emit(label, true, out);

	require_char("ERROR in process_if\nMISSING )\n", ')');
	out = statement(out, function);

	asprintf(&label, "JUMP %c_END_IF_%d\n:ELSE_%d\n", 37, number, number);
	out = emit(label, true, out);

	if(!strcmp(global_token->s, "else"))
	{
		global_token = global_token->next;
		out = statement(out, function);
	}
	asprintf(&label, ":_END_IF_%d\n", number);
	out = emit(label, true, out);
	return out;
}

int for_count;
struct token_list* process_for(struct token_list* out, struct token_list* function)
{
	char* label;
	int number = for_count;
	for_count = for_count + 1;

	asprintf(&label, "# FOR_initialization_%d\n", number);
	out = emit(label, true, out);

	global_token = global_token->next;

	require_char("ERROR in process_for\nMISSING (\n", '(');
	out = expression(out, function);

	asprintf(&label, ":FOR_%d\n", number);
	out = emit(label, true, out);

	require_char("ERROR in process_for\nMISSING ;1\n", ';');
	out = expression(out, function);

	asprintf(&label, "TEST\nJUMP_EQ %c%s_%d\nJUMP %c%s_%d\n:FOR_ITER_%d\n", 37, "FOR_END", number, 37, "FOR_THEN", number, number);
	out = emit(label, true, out);

	require_char("ERROR in process_for\nMISSING ;2\n", ';');
	out = expression(out, function);

	asprintf(&label, "JUMP %c%s_%d\n:FOR_THEN_%d\n", 37, "FOR", number, number);
	out = emit(label, true, out);

	require_char("ERROR in process_for\nMISSING )\n", ')');
	out = statement(out, function);

	asprintf(&label, "JUMP %c%s_ITER_%d\n:FOR_END_%d\n", 37, "FOR", number, number);
	out = emit(label, true, out);
	return out;
}

/* Process Assembly statements */
struct token_list* process_asm(struct token_list* out)
{
	global_token = global_token->next;
	require_char("ERROR in process_asm\nMISSING (\n", '(');
	while('"' == global_token->s[0])
	{
		out = emit((global_token->s + 1), true, out);
		out = emit("\n", true, out);
		global_token = global_token->next;
	}
	require_char("ERROR in process_asm\nMISSING )\n", ')');
	require_char("ERROR in process_asm\nMISSING ;\n", ';');
	return out;
}

/* Process while loops */
int while_count;
struct token_list* process_while(struct token_list* out, struct token_list* function)
{
	char* label;
	int number = while_count;
	while_count = while_count + 1;

	asprintf(&label, ":WHILE_%d\n", number);
	out = emit(label, true, out);

	global_token = global_token->next;

	require_char("ERROR in process_while\nMISSING (\n", '(');
	out = expression(out, function);

	asprintf(&label, "TEST\nJUMP_EQ %c%s_WHILE_%d\n# THEN_while_%d\n", 37, "END", number, number);
	out = emit(label, true, out);

	require_char("ERROR in process_while\nMISSING )\n", ')');
	out = statement(out, function);

	asprintf(&label, "JUMP %c%s_%d\n:END_WHILE_%d\n", 37, "WHILE", number, number);
	out = emit(label, true, out);
	return out;
}

/* Ensure that functions return */
struct token_list* return_result(struct token_list* out, struct token_list* function)
{
	global_token = global_token->next;
	if(global_token->s[0] != ';') out = expression(out, function);

	require_char("ERROR in return_result\nMISSING ;\n", ';');

	for(struct token_list* i = function->locals; NULL != i; i = i->locals)
	{
		char* label;
		asprintf(&label, "POP_ebx\t# %s\n", "_return_result_locals");
		out = emit(label, true, out);
		function->locals = function->locals->locals;
	}
	out = emit("RETURN\n", true, out);
	return out;
}

struct token_list* recursive_statement(struct token_list* out, struct token_list* function)
{
	global_token = global_token->next;
	struct token_list* frame = function->locals;

	while(strcmp(global_token->s, "}"))
	{
		out = statement(out, function);
	}
	global_token = global_token->next;

	/* Clean up any locals added */
	if(NULL != function->locals)
	{
		for(struct token_list* i = function->locals; frame != i; i = i->locals)
		{
			char* label;
			asprintf(&label, "POP_ebx\t# %s\n", "_recursive_statement_locals");
			out = emit(label, true, out);
			function->locals = function->locals->locals;
		}
	}
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
 *     asm ( "assembly" ... "assembly" ) ;
 *     return ;
 *     expr ;
 */
struct token_list* statement(struct token_list* out, struct token_list* function)
{
	if(global_token->s[0] == '{') out = recursive_statement(out, function);
	else if((!strcmp(global_token->s, "char")) | (!strcmp(global_token->s, "int"))) out = collect_local(out, function);
	else if(!strcmp(global_token->s, "if")) out = process_if(out, function);
	else if(!strcmp(global_token->s, "while")) out = process_while(out, function);
	else if(!strcmp(global_token->s, "for")) out = process_for(out, function);
	else if(!strcmp(global_token->s, "asm")) out = process_asm(out);
	else if(!strcmp(global_token->s, "return")) out = return_result(out, function);
	else
	{
		out = expression(out, function);
		require_char("ERROR in statement\nMISSING ;\n", ';');
	}
	return out;
}

/* Collect function arguments */
void collect_arguments(struct token_list* function)
{
	global_token = global_token->next;

	while(strcmp(global_token->s, ")"))
	{
		struct type* type_size = type_name();
		if(global_token->s[0] == ')')
		{ /* deal with foo(int,char) */
			if((!type_size->_void) || (type_size->indirect))
			{
				struct token_list* a = calloc(1, sizeof(struct token_list));
				a->arguments = function->arguments;
				function->arguments = a;
			}
		}
		else if(global_token->s[0] == ',')
		{ /* deal with foo(int,char) */
			struct token_list* a = calloc(1, sizeof(struct token_list));
			a->arguments = function->arguments;
			function->arguments = a;
			global_token = global_token->next;
		}
		else
		{
			struct token_list* a = sym_declare(global_token->s, ARGUEMENT, type_size);
			a->arguments = function->arguments;
			function->arguments = a;
			global_token = global_token->next;
		}

		/* ignore trailing comma (needed for foo(bar(), 1); expressions*/
		if(global_token->s[0] == ',') global_token = global_token->next;
	}
	global_token = global_token->next;
}

struct token_list* declare_global(struct token_list* out, struct type* type_size)
{
	char* label;
	asprintf(&label, "# Defining global %s\n:GLOBAL_%s\n", global_token->prev->s, global_token->prev->s);
	out = emit(label, true, out);
	sym_declare(global_token->prev->s, GLOBAL, type_size);

	global_token = global_token->next;
	return emit("NOP\n", true, out);
}

struct token_list* declare_function(struct token_list* out, struct type* type)
{
	char* label;
	asprintf(&label, "# Defining function %s\n:FUNCTION_%s\n", global_token->prev->s, global_token->prev->s);
	struct token_list* func = sym_declare(global_token->prev->s, FUNCTION, calloc(1, sizeof(struct type)));
	struct token_list* current = global_symbol_list;
	func->size = type;
	collect_arguments(func);

	if(global_token->s[0] != ';')
	{
		out = emit(label, true, out);
		out = statement(out, func);

		/* Prevent duplicate RETURNS */
		if(strcmp(out->s, "RETURN\n"))
		{
			out = emit("RETURN\n", true, out);
		}
	}
	else
	{
		global_token = global_token->next;
	}

	global_symbol_list = current;
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
		struct type* type_size = type_name();
		global_token = global_token->next;
		if(global_token->s[0] == ';') out = declare_global(out, type_size);
		else if(global_token->s[0] == '(') out = declare_function(out, type_size);
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
	free(i);
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
	fprintf(output, "\n# Program strings\n\n");
	recursive_output(output, strings);
	fclose(output);
	return 0;
}
