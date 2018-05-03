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

/* Global lists */
struct token_list* global_symbol_list;
struct token_list* global_function_list;
struct token_list* global_constant_list;

/* What we are currently working on */
struct token_list* break_locals;
struct type* current_target;
char* break_target;

/* Imported functions */
char* parse_string(char* string);
char* numerate_number(int a);
char* postpend_char(char* s, char a);
char* prepend_char(char a, char* s);
char* prepend_string(char* add, char* base);
void require_match(char* message, char* required);

struct token_list* emit(char *s, struct token_list* head)
{
	struct token_list* t = calloc(1, sizeof(struct token_list));
	t->next = head;
	t->s = s;
	return t;
}

struct token_list* sym_declare(char *s, struct type* t, struct token_list* list)
{
	struct token_list* a = calloc(1, sizeof(struct token_list));
	a->next = list;
	a->s = s;
	a->type = t;
	return a;
}

struct token_list* sym_lookup(char *s, struct token_list* symbol_list)
{
	struct token_list* i;
	for(i = symbol_list; NULL != i; i = i->next)
	{
		if(match(i->s, s)) return i;
	}
	return NULL;
}

int stack_index(struct token_list* a, struct token_list* function)
{
	int depth = 4 * function->temps;
	struct token_list* i;
	for(i = function->locals; NULL != i; i = i->next)
	{
		if(i == a) return depth;
		else depth = depth + 4;
	}

	/* Deal with offset caused by return pointer */
	depth = depth+ 4;

	for(i = function->arguments; NULL != i; i = i->next)
	{
		if(i == a)
		{
			if(match("main", function->s))
			{
				if(match("argc", i->s)) return (depth - 4);
				else if(match("argv", i->s)) return (depth + 4);
			}
			return depth;
		}
		else depth = depth + 4;
	}

	file_print(a->s,stderr);
	file_print(" does not exist in function ", stderr);
	file_print(function->s,stderr);
	file_print("\x0A",stderr);
	exit(EXIT_FAILURE);
}

struct token_list* expression(struct token_list* out, struct token_list* function);
struct token_list* function_call(struct token_list* out, struct token_list* function, char* s, int bool)
{
	require_match("ERROR in process_expression_list\nNo ( was found\x0A", "(");
	int passed = 0;

	if(global_token->s[0] != ')')
	{
		out = expression(out, function);
		out = emit("PUSH_eax\t#_process_expression1\x0A", out);
		function->temps = function->temps + 1;
		passed = 1;

		while(global_token->s[0] == ',')
		{
			global_token = global_token->next;
			out = expression(out, function);
			out = emit("PUSH_eax\t#_process_expression2\x0A", out);
			function->temps = function->temps + 1;
			passed = passed + 1;
		}
	}

	require_match("ERROR in process_expression_list\nNo ) was found\n", ")");

	if(bool)
	{
		struct token_list* a = sym_lookup(s, function->arguments);
		out = emit(prepend_string("LOAD_EFFECTIVE_ADDRESS %", numerate_number(stack_index(a, function))), out);
		out = emit("LOAD_INTEGER\n", out);
		out = emit("CALL_eax\n", out);
	}
	else
	{
		out = emit(prepend_string("CALL_IMMEDIATE %FUNCTION_", postpend_char(s, LF)), out);
	}

	for(; passed > 0; passed = passed - 1)
	{
		out = emit("POP_ebx\t# _process_expression_locals\x0A", out);
		function->temps = function->temps - 1;
	}
	return out;
}

struct token_list* sym_get_value(char *s, struct token_list* out, struct token_list* function)
{
	global_token = global_token->next;
	struct token_list* a = sym_lookup(s, global_constant_list);
	if(NULL != a)
	{
		out = emit(prepend_string("LOAD_IMMEDIATE_eax %", postpend_char(a->arguments->s, LF)), out); return out;
	}

	a= sym_lookup(s, function->locals);
	if(NULL != a)
	{
		current_target = a->type;
		out = emit(prepend_string("LOAD_EFFECTIVE_ADDRESS %", numerate_number(stack_index(a, function))), out);
		if(!match("=", global_token->s)) out = emit("LOAD_INTEGER\x0A", out);
		return out;
	}

	a = sym_lookup(s, function->arguments);
	if(NULL != a)
	{
		current_target = a->type;
		if(match("FUNCTION", a->type->name))
		{
			if(!match("(", global_token->s))
			{
				out = emit(prepend_string("#Loading address of function\nLOAD_EFFECTIVE_ADDRESS %", numerate_number(stack_index(a, function))), out);
				out = emit("LOAD_INTEGER\n", out);
				return out;
			}
			return function_call(out, function, s, TRUE);
		}
		out = emit(prepend_string("LOAD_EFFECTIVE_ADDRESS %", numerate_number(stack_index(a, function))), out);
		if(!match("=", global_token->s) && !match("argv", s)) out = emit("LOAD_INTEGER\x0A", out);
		return out;
	}

	a= sym_lookup(s, global_function_list);
	if(NULL != a)
	{
		if(!match("(", global_token->s))
		{
			out = emit(prepend_string("LOAD_IMMEDIATE_eax &FUNCTION_", postpend_char(s, LF)), out);
			return out;
		}
		else
		{
			return function_call(out, function, s, FALSE);
		}
	}

	a = sym_lookup(s, global_symbol_list);
	if(NULL != a)
	{
		current_target = a->type;
		out = emit(prepend_string("LOAD_IMMEDIATE_eax &GLOBAL_", postpend_char(s, LF)), out);
		if(!match("=", global_token->s)) out = emit("LOAD_INTEGER\x0A", out);
		return out;
	}

	file_print(s ,stderr);
	file_print(" is not a defined symbol\x0A", stderr);
	exit(EXIT_FAILURE);
}

/*
 * primary-expr:
 *     identifier
 *     constant
 *     ( expression )
 */
int string_num;
struct token_list* primary_expr(struct token_list* out, struct token_list* function)
{
	if(('0' <= global_token->s[0]) & (global_token->s[0] <= '9'))
	{
		out = emit(prepend_string("LOAD_IMMEDIATE_eax %", postpend_char(global_token->s, LF)), out);
		global_token = global_token->next;
	}
	else if((('a' <= global_token->s[0]) & (global_token->s[0] <= 'z')) | (('A' <= global_token->s[0]) & (global_token->s[0] <= 'Z')))
	{
		out = sym_get_value(global_token->s, out, function);
	}
	else if(global_token->s[0] == '(')
	{
		global_token = global_token->next;
		out = expression(out, function);
		require_match("Error in Primary expression\nDidn't get )\x0A", ")");
	}
	else if(global_token->s[0] == 39)
	{ /* 39 == ' */
		out = emit("LOAD_IMMEDIATE_eax %", out);
		out = emit(numerate_number(global_token->s[1]), out);
		global_token = global_token->next;
	}
	else if(global_token->s[0] == 34)
	{ /* 34 == " */
		char* number_string = numerate_number(string_num);
		out = emit("LOAD_IMMEDIATE_eax &STRING_", out);
		out = emit(number_string, out);

		/* The target */
		strings_list = emit(":STRING_", strings_list);
		strings_list = emit(number_string, strings_list);

		/* Parse the string */
		strings_list = emit(parse_string(global_token->s), strings_list);
		global_token = global_token->next;

		string_num = string_num + 1;
	}
	else
	{
		file_print("Recieved ", stderr);
		file_print(global_token->s, stderr);
		file_print(" in primary_expr\x0A", stderr);
		exit(EXIT_FAILURE);
	}

	return out;
}

struct type* last_type;

struct token_list* pre_recursion(struct token_list* out, struct token_list* func)
{
	last_type = current_target;
	global_token = global_token->next;
	out = emit("PUSH_eax\t#_common_recursion\x0A", out);
	func->temps = func->temps + 1;
	return out;
}

struct type* promote_type(struct type* a, struct type* b)
{
	if(NULL == a)
	{
		return b;
	}
	if(NULL == b)
	{
		return a;
	}

	struct type* i;
	for(i = global_types; NULL != i; i = i->next)
	{
		if(a->name == i->name)
		{
			return a;
		}
		if(b->name == i->name)
		{
			return b;
		}
		if(a->name == i->indirect->name)
		{
			return a;
		}
		if(b->name == i->indirect->name)
		{
			return b;
		}
	}
	return NULL;
}

struct token_list* post_recursion(struct token_list* out, struct token_list* func)
{
	current_target = promote_type(current_target, last_type);
	func->temps = func->temps - 1;
	out = emit("POP_ebx\t# _common_recursion\x0A", out);
	return out;
}

int ceil_log2(int a)
{
	int result = 0;
	if((a & (a - 1)) == 0)
	{
		result = -1;
	}

	while(a > 0)
	{
		result = result + 1;
		a = a >> 1;
	}

	return result;
}

/*
 * postfix-expr:
 *         primary-expr
 *         postfix-expr [ expression ]
 *         postfix-expr ( expression-list-opt )
 *         postfix-expr -> member
 */
struct token_list* postfix_expr(struct token_list* out, struct token_list* function)
{
	out = primary_expr(out, function);

	while(1)
	{
		if(global_token->s[0] == '[')
		{
			struct type* target = current_target;
			struct type* a = current_target;
			out = pre_recursion(out, function);
			out = expression(out, function);
			out = post_recursion(out, function);

			/* Add support for Ints */
			if( 1 != a->indirect->size)
			{
				out = emit(prepend_string("SAL_eax_Immediate8 !", numerate_number(ceil_log2(a->indirect->size))), out);
			}

			out = emit("ADD_ebx_to_eax\x0A", out);
			current_target = target;

			if(!match("=", global_token->next->s))
			{
				if( 4 == a->indirect->size)
				{
					out = emit("LOAD_INTEGER\x0A", out);
				}
				else
				{
					out = emit("LOAD_BYTE\x0A", out);
				}
			}
			require_match("ERROR in postfix_expr\nMissing ]\x0A", "]");
		}
		else if(match("->", global_token->s))
		{
			out = emit("# looking up offset\x0A", out);
			global_token = global_token->next;
			struct type* i;
			for(i = current_target->members; NULL != i; i = i->members)
			{
				if(match(i->name, global_token->s)) break;
			}
			if(NULL == i)
			{
				file_print("ERROR in postfix_expr ", stderr);
				file_print(current_target->name, stderr);
				file_print("->", stderr);
				file_print(global_token->s, stderr);
				file_print(" does not exist\x0A", stderr);
				exit(EXIT_FAILURE);
			}
			if(0 != i->offset)
			{
				out = emit("# -> offset calculation\x0A", out);
				out = emit(prepend_string("LOAD_IMMEDIATE_ebx %", numerate_number(i->offset)), out);
				out = emit("ADD_ebx_to_eax\x0A", out);
			}
			if(!match("=", global_token->next->s))
			{
				out = emit("LOAD_INTEGER\x0A", out);
			}
			 current_target = i->type;
			global_token = global_token->next;
		}
		else return out;
	}
}

/*
 * unary-expr:
 *         postfix-expr
 *         - postfix-expr
 *         !postfix-expr
 *         sizeof ( type )
 */
struct type* type_name();
struct token_list* unary_expr(struct token_list* out, struct token_list* function)
{
	if(match("-", global_token->s))
	{
		out = emit("LOAD_IMMEDIATE_eax %0\x0A", out);
		out = pre_recursion(out, function);
		out = postfix_expr(out, function);
		out = post_recursion(out, function);
		out = emit("SUBTRACT_eax_from_ebx_into_ebx\nMOVE_ebx_to_eax\x0A", out);
	}
	else if(match("!", global_token->s))
	{
		out = emit("LOAD_IMMEDIATE_eax %1\x0A", out);
		out = pre_recursion(out, function);
		out = postfix_expr(out, function);
		out = post_recursion(out, function);
		out = emit("XOR_ebx_eax_into_eax\x0A", out);
	}
	else if(match("sizeof", global_token->s))
	{
		global_token = global_token->next;
		require_match("ERROR in unary_expr\nMissing (\x0A", "(");
		struct type* a = type_name();
		require_match("ERROR in unary_expr\nMissing )\x0A", ")");

		out = emit(prepend_string("LOAD_IMMEDIATE_eax %", numerate_number(a->size)), out);
	}
	else out = postfix_expr(out, function);

	return out;
}

/*
 * additive-expr:
 *         postfix-expr
 *         additive-expr * postfix-expr
 *         additive-expr / postfix-expr
 *         additive-expr % postfix-expr
 *         additive-expr + postfix-expr
 *         additive-expr - postfix-expr
 */
struct token_list* additive_expr(struct token_list* out, struct token_list* function)
{
	out = unary_expr(out, function);

	while(1)
	{
		if(match("+", global_token->s))
		{
			out = pre_recursion(out, function);
			out = unary_expr(out, function);
			out = post_recursion(out, function);
			out = emit("ADD_ebx_to_eax\x0A", out);
		}
		else if(match("-", global_token->s))
		{
			out = pre_recursion(out, function);
			out = unary_expr(out, function);
			out = post_recursion(out, function);
			out = emit("SUBTRACT_eax_from_ebx_into_ebx\nMOVE_ebx_to_eax\x0A", out);
		}
		else if(match("*", global_token->s))
		{
			out = pre_recursion(out, function);
			out = unary_expr(out, function);
			out = post_recursion(out, function);
			out = emit("MULTIPLY_eax_by_ebx_into_eax\x0A", out);
		}
		else if(match("/", global_token->s))
		{
			out = pre_recursion(out, function);
			out = unary_expr(out, function);
			out = post_recursion(out, function);
			out = emit("XCHG_eax_ebx\nLOAD_IMMEDIATE_edx %0\nDIVIDE_eax_by_ebx_into_eax\x0A", out);
		}
		else if(match("%", global_token->s))
		{
			out = pre_recursion(out, function);
			out = unary_expr(out, function);
			out = post_recursion(out, function);
			out = emit("XCHG_eax_ebx\nLOAD_IMMEDIATE_edx %0\nMODULUS_eax_from_ebx_into_ebx\nMOVE_edx_to_eax\x0A", out);
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
		if(match("<<", global_token->s))
		{
			out = pre_recursion(out, function);
			out = additive_expr(out, function);
			out = post_recursion(out, function);
			/* Ugly hack to Work around flaw in x86 */
			struct token_list* old = out->next;
			free(out);
			out = emit("COPY_eax_to_ecx\nPOP_eax\nSAL_eax_cl\x0A", old);
		}
		else if(match(">>", global_token->s))
		{
			out = pre_recursion(out, function);
			out = additive_expr(out, function);
			out = post_recursion(out, function);
			/* Ugly hack to Work around flaw in x86 */
			struct token_list* old = out->next;
			free(out);
			out = emit("COPY_eax_to_ecx\nPOP_eax\nSAR_eax_cl\x0A", old);
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
 *         relational-expr < shift-expr
 *         relational-expr <= shift-expr
 *         relational-expr >= shift-expr
 *         relational-expr > shift-expr
 */
struct token_list* relational_expr(struct token_list* out, struct token_list* function)
{
	out = shift_expr(out, function);

	while(1)
	{
		if(match("<", global_token->s))
		{
			out = pre_recursion(out, function);
			out = shift_expr(out, function);
			out = post_recursion(out, function);
			out = emit("CMP\nSETL\nMOVEZBL\x0A", out);
		}
		else if(match("<=", global_token->s))
		{
			out = pre_recursion(out, function);
			out = shift_expr(out, function);
			out = post_recursion(out, function);
			out = emit("CMP\nSETLE\nMOVEZBL\x0A", out);
		}
		else if(match(">=", global_token->s))
		{
			out = pre_recursion(out, function);
			out = shift_expr(out, function);
			out = post_recursion(out, function);
			out = emit("CMP\nSETGE\nMOVEZBL\x0A", out);
		}
		else if(match(">", global_token->s))
		{
			out = pre_recursion(out, function);
			out = shift_expr(out, function);
			out = post_recursion(out, function);
			out = emit("CMP\nSETG\nMOVEZBL\x0A", out);
		}
		else return out;
	}
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
		if(match("==", global_token->s))
		{
			out = pre_recursion(out, function);
			out = relational_expr(out, function);
			out = post_recursion(out, function);
			out = emit("CMP\nSETE\nMOVEZBL\x0A", out);
		}
		else if(match("!=", global_token->s))
		{
			out = pre_recursion(out, function);
			out = relational_expr(out, function);
			out = post_recursion(out, function);
			out = emit("CMP\nSETNE\nMOVEZBL\x0A", out);
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
		out = pre_recursion(out, function);
		out = equality_expr(out, function);
		out = post_recursion(out, function);
		out = emit("AND_eax_ebx\x0A", out);
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
		out = pre_recursion(out, function);
		out = bitwise_and_expr(out, function);
		out = post_recursion(out, function);
		out = emit("OR_eax_ebx\x0A", out);
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
		struct type* target = current_target;
		int member = match("]", global_token->prev->s);
		out = pre_recursion(out, function);
		out = expression(out, function);
		out = post_recursion(out, function);

		if(member)
		{
			if(1 == target->indirect->size) out = emit("STORE_CHAR\x0A", out);
			else if(4 == target->indirect->size)
			{
				out = emit("STORE_INTEGER\x0A", out);
			}
		}
		else
		{
			out = emit("STORE_INTEGER\x0A", out);
		}
	}
	return out;
}


/* Process local variable */
struct token_list* collect_local(struct token_list* out, struct token_list* function)
{
	struct type* type_size = type_name();
	out = emit(prepend_string("# Defining local ", prepend_string(global_token->s, "\x0A")), out);

	struct token_list* a = sym_declare(global_token->s, type_size, function->locals);
	function->locals = a;
	global_token = global_token->next;
	function->temps = function->temps - 1;

	if(global_token->s[0] == '=')
	{
		global_token = global_token->next;
		out = expression(out, function);
	}
	function->temps = function->temps + 1;

	require_match("ERROR in collect_local\nMissing ;\x0A", ";");

	out = emit(prepend_string("PUSH_eax\t#", prepend_string(a->s, "\x0A")), out);
	return out;
}

struct token_list* statement(struct token_list* out, struct token_list* function);

/* Evaluate if statements */
int if_count;
struct token_list* process_if(struct token_list* out, struct token_list* function)
{
	char* number_string = numerate_number(if_count);
	if_count = if_count + 1;

	out = emit(prepend_string("# IF_", number_string), out);

	global_token = global_token->next;
	require_match("ERROR in process_if\nMISSING (\x0A", "(");
	out = expression(out, function);

	out = emit(prepend_string("TEST\nJUMP_EQ %ELSE_", number_string), out);

	require_match("ERROR in process_if\nMISSING )\x0A", ")");
	out = statement(out, function);

	out = emit(prepend_string("JUMP %_END_IF_", number_string), out);
	out = emit(prepend_string(":ELSE_", number_string), out);

	if(match("else", global_token->s))
	{
		global_token = global_token->next;
		out = statement(out, function);
	}
	out = emit(prepend_string(":_END_IF_", number_string), out);
	return out;
}

int for_count;
struct token_list* process_for(struct token_list* out, struct token_list* function)
{
	char* number_string = numerate_number(for_count);
	for_count = for_count + 1;

	char* nested_break = break_target;
	struct token_list* nested_locals = break_locals;
	break_locals = function->locals;
	break_target = prepend_string("FOR_END_", number_string);

	out = emit(prepend_string("# FOR_initialization_", number_string), out);

	global_token = global_token->next;

	require_match("ERROR in process_for\nMISSING (\x0A", "(");
	if(!match(";",global_token->s))
	{
		out = expression(out, function);
	}

	out = emit(prepend_string(":FOR_", number_string), out);

	require_match("ERROR in process_for\nMISSING ;1\x0A", ";");
	out = expression(out, function);

	out = emit(prepend_string("TEST\nJUMP_EQ %FOR_END_", number_string), out);
	out = emit(prepend_string("JUMP %FOR_THEN_", number_string), out);
	out = emit(prepend_string(":FOR_ITER_", number_string), out);

	require_match("ERROR in process_for\nMISSING ;2\x0A", ";");
	out = expression(out, function);

	out = emit(prepend_string("JUMP %FOR_", number_string), out);
	out = emit(prepend_string(":FOR_THEN_", number_string), out);

	require_match("ERROR in process_for\nMISSING )\x0A", ")");
	out = statement(out, function);

	out = emit(prepend_string("JUMP %FOR_ITER_", number_string), out);
	out = emit(prepend_string(":FOR_END_", number_string), out);

	break_target = nested_break;
	break_locals = nested_locals;
	return out;
}

/* Process Assembly statements */
struct token_list* process_asm(struct token_list* out)
{
	global_token = global_token->next;
	require_match("ERROR in process_asm\nMISSING (\x0A", "(");
	while(34 == global_token->s[0])
	{/* 34 == " */
		out = emit((global_token->s + 1), out);
		out = emit("\x0A", out);
		global_token = global_token->next;
	}
	require_match("ERROR in process_asm\nMISSING )\x0A", ")");
	require_match("ERROR in process_asm\nMISSING ;\x0A", ";");
	return out;
}

/* Process do while loops */
int do_count;
struct token_list* process_do(struct token_list* out, struct token_list* function)
{
	char* number_string = numerate_number(do_count);
	do_count = do_count + 1;

	char* nested_break = break_target;
	struct token_list* nested_locals = break_locals;
	break_locals = function->locals;
	break_target = prepend_string("DO_END_", number_string);

	out = emit(prepend_string(":DO_", number_string), out);

	global_token = global_token->next;
	out = statement(out, function);

	require_match("ERROR in process_do\nMISSING while\x0A", "while");
	require_match("ERROR in process_do\nMISSING (\x0A", "(");
	out = expression(out, function);
	require_match("ERROR in process_do\nMISSING )\x0A", ")");
	require_match("ERROR in process_do\nMISSING ;\x0A", ";");

	out = emit(prepend_string("TEST\nJUMP_NE %DO_", number_string), out);
	out = emit(prepend_string(":DO_END_", number_string), out);

	break_locals = nested_locals;
	break_target = nested_break;
	return out;
}


/* Process while loops */
int while_count;
struct token_list* process_while(struct token_list* out, struct token_list* function)
{
	char* number_string = numerate_number(while_count);
	while_count = while_count + 1;

	char* nested_break = break_target;
	struct token_list* nested_locals = break_locals;
	break_locals = function->locals;

	break_target = prepend_string("END_WHILE_", number_string);

	out = emit(prepend_string(":WHILE_", number_string), out);

	global_token = global_token->next;
	require_match("ERROR in process_while\nMISSING (\x0A", "(");
	out = expression(out, function);

	out = emit(prepend_string("TEST\nJUMP_EQ %END_WHILE_", number_string), out);
	out = emit(prepend_string("# THEN_while_", number_string), out);

	require_match("ERROR in process_while\nMISSING )\x0A", ")");
	out = statement(out, function);

	out = emit(prepend_string("JUMP %WHILE_", number_string), out);
	out = emit(prepend_string(":END_WHILE_", number_string), out);

	break_locals = nested_locals;
	break_target = nested_break;
	return out;
}

/* Ensure that functions return */
struct token_list* return_result(struct token_list* out, struct token_list* function)
{
	global_token = global_token->next;
	if(global_token->s[0] != ';') out = expression(out, function);

	require_match("ERROR in return_result\nMISSING ;\x0A", ";");

	struct token_list* i;
	for(i = function->locals; NULL != i; i = i->next)
	{
		out = emit("POP_ebx\t# _return_result_locals\x0A", out);
	}
	out = emit("RETURN\x0A", out);
	return out;
}

struct token_list* recursive_statement(struct token_list* out, struct token_list* function)
{
	global_token = global_token->next;
	struct token_list* frame = function->locals;

	while(!match("}", global_token->s))
	{
		out = statement(out, function);
	}
	global_token = global_token->next;

	/* Clean up any locals added */
	if(NULL != function->locals)
	{
		struct token_list* i;
		for(i = function->locals; frame != i; i = i->next)
		{
			if(!match("RETURN\x0A", out->s))
			{
				out = emit( "POP_ebx\t# _recursive_statement_locals\x0A", out);
			}
			function->locals = function->locals->next;
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
 *     do statement while ( expression ) ;
 *     while ( expression ) statement
 *     for ( expression ; expression ; expression ) statement
 *     asm ( "assembly" ... "assembly" ) ;
 *     goto label ;
 *     label:
 *     return ;
 *     break ;
 *     expr ;
 */

struct type* lookup_type(char* s);
struct token_list* statement(struct token_list* out, struct token_list* function)
{
	if(global_token->s[0] == '{')
	{
		out = recursive_statement(out, function);
	}
	else if(':' == global_token->s[0])
	{
		out = emit(global_token->s, out);
		out = emit("\t#C goto label\x0A", out);
		global_token = global_token->next;
	}
	else if((NULL != lookup_type(global_token->s)) || match("struct", global_token->s))
	{
		out = collect_local(out, function);
	}
	else if(match("if", global_token->s))
	{
		out = process_if(out, function);
	}
	else if(match("do", global_token->s))
	{
		out = process_do(out, function);
	}
	else if(match("while", global_token->s))
	{
		out = process_while(out, function);
	}
	else if(match("for", global_token->s))
	{
		out = process_for(out, function);
	}
	else if(match("asm", global_token->s))
	{
		out = process_asm(out);
	}
	else if(match("goto", global_token->s))
	{
		global_token = global_token->next;
		out = emit(prepend_string("JUMP %", prepend_string(global_token->s, "\x0A")), out);
		global_token = global_token->next;
		require_match("ERROR in statement\nMissing ;\x0A", ";");
	}
	else if(match("return", global_token->s))
	{
		out = return_result(out, function);
	}
	else if(match("break", global_token->s))
	{
		if(NULL == break_target)
		{
			file_print("Not inside of a loop or case statement", stderr);
			exit(EXIT_FAILURE);
		}
		struct token_list* i = function->locals;
		while(i != break_locals)
		{
			if(NULL == i) break;
			out = emit("POP_ebx\t# break_cleanup_locals\x0A", out);
			i = i->next;
		}
		global_token = global_token->next;
		out = emit(prepend_string("JUMP %", prepend_string(break_target, "\x0A")), out);
		require_match("ERROR in statement\nMissing ;\x0A", ";");
	}
	else
	{
		out = expression(out, function);
		require_match("ERROR in statement\nMISSING ;\x0A", ";");
	}
	return out;
}

/* Collect function arguments */
void collect_arguments(struct token_list* function)
{
	global_token = global_token->next;

	while(!match(")", global_token->s))
	{
		struct type* type_size = type_name();
		if(global_token->s[0] == ')')
		{
			/* deal with foo(int|char|void) */
			global_token = global_token->prev;
		}
		else if(global_token->s[0] != ',')
		{
			/* deal with foo(int a, char b) */
			struct token_list* a = sym_declare(global_token->s, type_size, function->arguments);
			function->arguments = a;
		}

		/* foo(int,char,void) doesn't need anything done */
		global_token = global_token->next;

		/* ignore trailing comma (needed for foo(bar(), 1); expressions*/
		if(global_token->s[0] == ',') global_token = global_token->next;
	}
	global_token = global_token->next;
}

struct token_list* declare_function(struct token_list* out, struct type* type)
{
	char* essential = global_token->prev->s;
	struct token_list* func = sym_declare(global_token->prev->s, calloc(1, sizeof(struct type)), global_function_list);
	func->type = type;
	collect_arguments(func);

	/* allow previously defined functions to be looked up */
	global_function_list = func;

	/* If just a prototype don't waste time */
	if(global_token->s[0] == ';') global_token = global_token->next;
	else
	{
		out = emit(prepend_string("# Defining function ", prepend_string(essential, "\x0A")), out);
		out = emit(prepend_string(":FUNCTION_", prepend_string(essential, "\x0A")), out);
		out = statement(out, func);

		/* Prevent duplicate RETURNS */
		if(!match("RETURN\x0A", out->s))
		{
			out = emit("RETURN\x0A", out);
		}
	}
	return out;
}

/*
 * program:
 *     declaration
 *     declaration program
 *
 * declaration:
 *     CONSTANT identifer value
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
	struct type* type_size;
	while(NULL != global_token)
	{
new_type:
		if(match("CONSTANT", global_token->s))
		{
			global_constant_list =  sym_declare(global_token->next->s, NULL, global_constant_list);
			global_constant_list->arguments = global_token->next->next;
			global_token = global_token->next->next->next;
		}
		else
		{
			type_size = type_name();
			if(NULL == type_size)
			{
				goto new_type;
			}
			global_token = global_token->next;
			if(global_token->s[0] == ';')
			{
				/* Add to global symbol table */
				global_symbol_list = sym_declare(global_token->prev->s, type_size, global_symbol_list);

				/* Ensure 4 bytes are allocated for the global */
				globals_list = emit(prepend_string(":GLOBAL_", prepend_string(global_token->prev->s, "\x0A")), globals_list);
				globals_list = emit("NOP\x0A", globals_list);

				global_token = global_token->next;
			}
			else if(global_token->s[0] == '(') out = declare_function(out, type_size);
			else
			{
				file_print("Recieved ", stderr);
				file_print(global_token->s, stderr);
				file_print(" in program\x0A", stderr);
				exit(EXIT_FAILURE);
			}
		}
	}
	return out;
}

void recursive_output(struct token_list* i, FILE* out)
{
	if(NULL == i) return;
	recursive_output(i->next, out);
	file_print(i->s, out);
}
