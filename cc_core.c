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
struct type* global_types;
struct token_list* global_symbol_list;
struct token_list* global_function_list;
struct token_list* global_constant_list;

/* What we are currently working on */
struct token_list* current_target;

/* Imported functions */
char* parse_string(char* string);

struct token_list* emit(char *s, struct token_list* head)
{
	struct token_list* t = calloc(1, sizeof(struct token_list));
	t->next = head;
	t->s = s;
	return t;
}

struct token_list* double_emit(char* a, char* b, struct token_list* out, int flag)
{
	out = emit(a, out);
	out = emit(b, out);
	if(flag) out = emit("\n", out);
	return out;
}

char* numerate_number(int a)
{
	char* result = calloc(16, sizeof(char));
	int i = 0;

	/* Deal with Zero case */
	if(0 == a)
	{
		result[0] = '0';
		result[1] = '\n';
		return result;
	}

	/* Deal with negatives */
	if(0 > a)
	{
		result[0] = '-';
		i = 1;
		a = a * -1;
	}

	/* Using the largest 10^n number possible in 32bits */
	int divisor = 0x3B9ACA00;
	/* Skip leading Zeros */
	while(0 == (a / divisor)) divisor = divisor / 10;

	/* Now simply collect numbers until divisor is gone */
	while(0 < divisor)
	{
		result[i] = ((a / divisor) + 48);
		a = a % divisor;
		divisor = divisor / 10;
		i = i + 1;
	}

	result[i] = '\n';
	return result;
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
	for(struct token_list* i = symbol_list; NULL != i; i = i->next)
	{
		if(0 == strcmp(s,i->s)) return i;
	}
	return NULL;
}

int stack_index(struct token_list* a, struct token_list* function)
{
	int depth = 4 * function->temps;
	for(struct token_list* i = function->locals; NULL != i; i = i->next)
	{
		if(i == a) return depth;
		else depth = depth + 4;
	}

	/* Deal with offset caused by return pointer */
	depth = depth+ 4;

	for(struct token_list* i = function->arguments; NULL != i; i = i->next)
	{
		if(i == a) return depth;
		else depth = depth + 4;
	}

	fprintf(stderr, "%s does not exist in function %s\n", a->s, function->s);
	exit(EXIT_FAILURE);
}

struct token_list* sym_get_value(char *s, struct token_list* out, struct token_list* function)
{
	global_token = global_token->next;
	struct token_list* a = sym_lookup(s, global_constant_list);
	if(NULL != a)
	{
		out = double_emit("LOAD_IMMEDIATE_eax %", a->arguments->s, out, true); return out;
	}

	a= sym_lookup(s, global_function_list);
	if(NULL != a)
	{
		return out;
	}

	a= sym_lookup(s, function->locals);
	if(NULL != a)
	{
		current_target = a;
		out = double_emit("LOAD_EFFECTIVE_ADDRESS %", numerate_number(stack_index(a, function)), out, false);
		if(strcmp(global_token->s, "=")) out = emit("LOAD_INTEGER\n", out);
		return out;
	}
	a = sym_lookup(s, function->arguments);

	if(NULL != a)
	{
		current_target = a;
		out = double_emit("LOAD_EFFECTIVE_ADDRESS %", numerate_number(stack_index(a, function)), out, false);
		if(strcmp(global_token->s, "=")) out = emit("LOAD_INTEGER\n", out);
		return out;
	}

	a = sym_lookup(s, global_symbol_list);
	if(NULL != a)
	{
		current_target = a;
		out = double_emit("LOAD_IMMEDIATE_eax &GLOBAL_", s, out, true);
		if(strcmp(global_token->s, "=")) out = emit("LOAD_INTEGER\n", out);
		return out;
	}

	fprintf(stderr, "%s is not a defined symbol\n", s);
	exit(EXIT_FAILURE);
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

/*
 * primary-expr:
 *     identifier
 *     constant
 *     ( expression )
 */
struct token_list* primary_expr(struct token_list* out, struct token_list* function)
{
	if(('0' <= global_token->s[0]) & (global_token->s[0] <= '9'))
	{
		out = double_emit("LOAD_IMMEDIATE_eax %", global_token->s, out, true);
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
		require_char("Error in Primary expression\nDidn't get )\n", ')');
	}
	else if(global_token->s[0] == '\'')
	{
		out = emit("LOAD_IMMEDIATE_eax %", out);
		out = emit(numerate_number(global_token->s[1]), out);
		global_token = global_token->next;
	}
	else if(global_token->s[0] == '"')
	{
		static int string_num;
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
		fprintf(stderr, "Recieved %s in primary_expr\n", global_token->s);
		exit(EXIT_FAILURE);
	}

	return out;
}

/* Deal with Expression lists */
struct token_list* process_expression_list(struct token_list* out, struct token_list* function)
{
	char* func = global_token->prev->s;
	global_token = global_token->next;
	int temp = function->temps;

	if(global_token->s[0] != ')')
	{
		out = expression(out, function);
		out = emit("PUSH_eax\t#_process_expression1\n", out);
		function->temps = function->temps + 1;

		while(global_token->s[0] == ',')
		{
			global_token = global_token->next;
			out = expression(out, function);
			out = emit("PUSH_eax\t#_process_expression2\n", out);
			function->temps = function->temps + 1;
		}
		require_char("ERROR in process_expression_list\nNo ) was found\n", ')');
	}
	else global_token = global_token->next;

	out = double_emit("CALL_IMMEDIATE %FUNCTION_", func, out, true);

	for(int i = function->temps - temp; 0 != i; i = i - 1)
	{
		out = emit("POP_ebx\t# _process_expression_locals\n", out);
	}

	function->temps = temp;
	return out;
}

struct token_list* common_recursion(struct token_list* (*function) (struct token_list*, struct token_list*), struct token_list* out, struct token_list* func)
{
	global_token = global_token->next;
	out = emit("PUSH_eax\t#_common_recursion\n", out);
	func->temps = func->temps + 1;
	out = function(out, func);
	func->temps = func->temps - 1;
	out = emit("POP_ebx\t# _common_recursion\n", out);
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
			struct token_list* target = current_target;
			struct type* a = current_target->type;
			out = common_recursion(expression, out, function);

			/* Add support for Ints */
			if( 1 != a->indirect->size)
			{
				out = double_emit("SAL_eax_Immediate8 !", numerate_number(ceil_log2(a->indirect->size)), out, false);
			}

			out = emit("ADD_ebx_to_eax\n", out);
			current_target = target;

			if(strcmp(global_token->next->s, "="))
			{
				if( 4 == a->indirect->size)
				{
					out = emit("LOAD_INTEGER\n", out);
				}
				else
				{
					out = emit("LOAD_BYTE\n", out);
				}
			}
			require_char("ERROR in postfix_expr\nMissing ]\n", ']');
		}
		else if(global_token->s[0] == '(')
		{
			out = process_expression_list(out, function);
		}
		else if(!strcmp("->", global_token->s))
		{
			out = emit("# looking up offset\n", out);
			global_token = global_token->next;
			struct type* i;
			for(i = current_target->type->members; NULL != i; i = i->members)
			{
				if(!strcmp(i->name, global_token->s)) break;
			}
			if(NULL == i)
			{
				fprintf(stderr, "ERROR in postfix_expr %s->%s does not exist\n", current_target->type->name, global_token->s);
				exit(EXIT_FAILURE);
			}
			if(0 != i->offset)
			{
				out = emit("# -> offset calculation\n", out);
				out = double_emit("LOAD_IMMEDIATE_ebx %", numerate_number(i->offset), out, false);
				out = emit("ADD_ebx_to_eax\n", out);
			}
			if(strcmp(global_token->next->s, "="))
			{
				out = emit("LOAD_INTEGER\n", out);
			}
			global_token = global_token->next;
		}
		else return out;
	}
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
			out = emit("ADD_ebx_to_eax\n", out);
		}
		else if(global_token->s[0] == '-')
		{
			out = common_recursion(postfix_expr, out, function);
			out = emit("SUBTRACT_eax_from_ebx_into_ebx\nMOVE_ebx_to_eax\n", out);
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
			out = emit("COPY_eax_to_ecx\nPOP_eax\nSAL_eax_cl\n", old);
		}
		else if(!strcmp(global_token->s, ">>"))
		{
			out = common_recursion(additive_expr, out, function);
			// Ugly hack to Work around flaw in x86
			struct token_list* old = out->next;
			free(out);
			out = emit("COPY_eax_to_ecx\nPOP_eax\nSAR_eax_cl\n", old);
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
		if(!strcmp(global_token->s, "<"))
		{
			out = common_recursion(shift_expr, out, function);
			out = emit("CMP\nSETL\nMOVEZBL\n", out);
		}
		else if(!strcmp(global_token->s, "<="))
		{
			out = common_recursion(shift_expr, out, function);
			out = emit("CMP\nSETLE\nMOVEZBL\n", out);
		}
		else if(!strcmp(global_token->s, ">="))
		{
			out = common_recursion(shift_expr, out, function);
			out = emit("CMP\nSETGE\nMOVEZBL\n", out);
		}
		else if(!strcmp(global_token->s, ">"))
		{
			out = common_recursion(shift_expr, out, function);
			out = emit("CMP\nSETG\nMOVEZBL\n", out);
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
		if(!strcmp(global_token->s, "=="))
		{
			out = common_recursion(relational_expr, out, function);
			out = emit("CMP\nSETE\nMOVEZBL\n", out);
		}
		else if(!strcmp(global_token->s, "!="))
		{
			out = common_recursion(relational_expr, out, function);
			out = emit("CMP\nSETNE\nMOVEZBL\n", out);
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
		out = emit("AND_eax_ebx\n", out);
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
		out = emit("OR_eax_ebx\n", out);
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
			if(1 == target->type->indirect->size) out = emit("STORE_CHAR\n", out);
			else if(4 == target->type->indirect->size)
			{
				out = emit("STORE_INTEGER\n", out);
			}
		}
		else
		{
			out = emit("STORE_INTEGER\n", out);
		}
	}
	return out;
}

struct type* lookup_type(char* s)
{
	for(struct type* i = global_types; NULL != i; i = i->next)
	{
		if(!strcmp(i->name, s))
		{
			return i;
		}
	}
	return NULL;
}

struct type* type_name();
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
	require_char("ERROR in create_struct\nMissing {\n", '{');
	struct type* last = NULL;
	while('}' != global_token->s[0])
	{
		struct type* member_type = type_name();
		i = calloc(1, sizeof(struct type));
		i->name = global_token->s;
		i->members = last;
		i->size = member_type->size;
		i->offset = offset;
		offset = offset + member_type->size;
		global_token = global_token->next;
		require_char("ERROR in create_struct\nMissing ;\n", ';');
		last = i;
	}

	global_token = global_token->next;
	require_char("ERROR in create_struct\nMissing ;\n", ';');

	head->size = offset;
	head->members = last;
	head->indirect->members = last;
}


/*
 * type-name:
 *     char *
 *     int
 */
struct type* type_name()
{
	int structure = false;

	if(!strcmp(global_token->s, "struct"))
	{
		structure = true;
		global_token = global_token->next;
	}

	struct type* ret = lookup_type(global_token->s);

	if(NULL == ret && !structure)
	{
		fprintf(stderr, "Unknown type %s\n", global_token->s);
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

/* Process local variable */
struct token_list* collect_local(struct token_list* out, struct token_list* function)
{
	struct type* type_size = type_name();
	out = double_emit("# Defining local ", global_token->s, out, true);

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

	require_char("ERROR in collect_local\nMissing ;\n", ';');

	out = double_emit("PUSH_eax\t#", a->s, out, true);
	return out;
}

struct token_list* statement(struct token_list* out, struct token_list* function);

/* Evaluate if statements */
int if_count;
struct token_list* process_if(struct token_list* out, struct token_list* function)
{
	char* number_string = numerate_number(if_count);
	if_count = if_count + 1;

	out = double_emit("# IF_",number_string, out, false);

	global_token = global_token->next;
	require_char("ERROR in process_if\nMISSING (\n", '(');
	out = expression(out, function);

	out = double_emit("TEST\nJUMP_EQ %ELSE_", number_string, out, false);

	require_char("ERROR in process_if\nMISSING )\n", ')');
	out = statement(out, function);

	out = double_emit("JUMP %_END_IF_", number_string, out, false);
	out = double_emit(":ELSE_", number_string, out, false);

	if(!strcmp(global_token->s, "else"))
	{
		global_token = global_token->next;
		out = statement(out, function);
	}
	out = double_emit(":_END_IF_", number_string, out, false);
	return out;
}

int for_count;
struct token_list* process_for(struct token_list* out, struct token_list* function)
{
	char* number_string = numerate_number(for_count);
	for_count = for_count + 1;

	out = double_emit("# FOR_initialization_", number_string, out, false);

	global_token = global_token->next;

	require_char("ERROR in process_for\nMISSING (\n", '(');
	out = expression(out, function);

	out = double_emit(":FOR_", number_string, out , false);

	require_char("ERROR in process_for\nMISSING ;1\n", ';');
	out = expression(out, function);

	out = double_emit("TEST\nJUMP_EQ %FOR_END_", number_string, out, false);
	out = double_emit("JUMP %FOR_THEN_", number_string, out, false);
	out = double_emit(":FOR_ITER_", number_string, out, false);

	require_char("ERROR in process_for\nMISSING ;2\n", ';');
	out = expression(out, function);

	out = double_emit("JUMP %FOR_", number_string, out, false);
	out = double_emit(":FOR_THEN_", number_string, out, false);

	require_char("ERROR in process_for\nMISSING )\n", ')');
	out = statement(out, function);

	out = double_emit("JUMP %FOR_ITER_", number_string, out, false);
	out = double_emit(":FOR_END_", number_string, out, false);
	return out;
}

/* Process Assembly statements */
struct token_list* process_asm(struct token_list* out)
{
	global_token = global_token->next;
	require_char("ERROR in process_asm\nMISSING (\n", '(');
	while('"' == global_token->s[0])
	{
		out = emit((global_token->s + 1), out);
		out = emit("\n", out);
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
	char* number_string = numerate_number(while_count);
	while_count = while_count + 1;

	out = double_emit(":WHILE_", number_string, out, false);

	global_token = global_token->next;
	require_char("ERROR in process_while\nMISSING (\n", '(');
	out = expression(out, function);

	out = double_emit("TEST\nJUMP_EQ %END_WHILE_", number_string, out, false);
	out = double_emit("# THEN_while_", number_string, out, false);

	require_char("ERROR in process_while\nMISSING )\n", ')');
	out = statement(out, function);

	out = double_emit("JUMP %WHILE_", number_string, out, false);
	out = double_emit(":END_WHILE_", number_string, out, false);
	return out;
}

/* Ensure that functions return */
struct token_list* return_result(struct token_list* out, struct token_list* function)
{
	global_token = global_token->next;
	if(global_token->s[0] != ';') out = expression(out, function);

	require_char("ERROR in return_result\nMISSING ;\n", ';');

	for(struct token_list* i = function->locals; NULL != i; i = i->next)
	{
		out = emit("POP_ebx\t# _return_result_locals\n", out);
		function->locals = function->locals->next;
	}
	out = emit("RETURN\n", out);
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
		for(struct token_list* i = function->locals; frame != i; i = i->next)
		{
			out = emit( "POP_ebx\t# _recursive_statement_locals\n", out);
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
 *     while ( expression ) statement
 *     for ( expression ; expression ; expression ) statement
 *     asm ( "assembly" ... "assembly" ) ;
 *     return ;
 *     expr ;
 */
struct token_list* statement(struct token_list* out, struct token_list* function)
{
	if(global_token->s[0] == '{')
	{
		out = recursive_statement(out, function);
	}
	else if((NULL != lookup_type(global_token->s)) || !strcmp("struct", global_token->s))
	{
		out = collect_local(out, function);
	}
	else if(!strcmp(global_token->s, "if"))
	{
		out = process_if(out, function);
	}
	else if(!strcmp(global_token->s, "while"))
	{
		out = process_while(out, function);
	}
	else if(!strcmp(global_token->s, "for"))
	{
		out = process_for(out, function);
	}
	else if(!strcmp(global_token->s, "asm"))
	{
		out = process_asm(out);
	}
	else if(!strcmp(global_token->s, "return"))
	{
		out = return_result(out, function);
	}
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
		out = double_emit("# Defining function ", essential, out, true);
		out = double_emit(":FUNCTION_", essential, out, true);
		out = statement(out, func);

		/* Prevent duplicate RETURNS */
		if(strcmp(out->s, "RETURN\n"))
		{
			out = emit("RETURN\n", out);
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
	while(NULL != global_token)
	{
new_type:
		if(!strcmp(global_token->s, "CONSTANT"))
		{
			global_constant_list =  sym_declare(global_token->next->s, NULL, global_constant_list);
			global_constant_list->arguments = global_token->next->next;
			global_token = global_token->next->next->next;
		}
		else
		{
			struct type* type_size = type_name();
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
				globals_list = double_emit(":GLOBAL_", global_token->prev->s, globals_list, true);
				globals_list = emit("NOP\n", globals_list);

				global_token = global_token->next;
			}
			else if(global_token->s[0] == '(') out = declare_function(out, type_size);
			else
			{
				fprintf(stderr, "Recieved %s in program\n", global_token->s);
				exit(EXIT_FAILURE);
			}
		}
	}
	return out;
}

void recursive_output(FILE* out, struct token_list* i)
{
	if(NULL == i) return;
	recursive_output(out, i->next);
	fprintf(out, "%s", i->s);
}

/* Initialize default types */
void initialize_types()
{
	/* Define void */
	global_types = calloc(1, sizeof(struct type));
	global_types->name = "void";
	global_types->size = 4;
	/* void* has the same properties as void */
	global_types->indirect = global_types;

	/* Define int */
	struct type* a = calloc(1, sizeof(struct type));
	a->name = "int";
	a->size = 4;
	/* int* has the same properties as int */
	a->indirect = a;

	/* Define char* */
	struct type* b = calloc(1, sizeof(struct type));
	b->name = "char*";
	b->size = 4;

	/* Define char */
	struct type* c = calloc(1, sizeof(struct type));
	c->name = "char";
	c->size = 1;

	/* char** is char */
	c->indirect = b;
	b->indirect = c;

	/* Finalize type list */
	a->next = c;
	global_types->next = a;
}
