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
#include "gcc_req.h"
#include <stdint.h>

/* Global lists */
struct token_list* global_symbol_list;
struct token_list* global_function_list;
struct token_list* global_constant_list;

/* What we are currently working on */
struct type* current_target;
char* break_target_head;
char* break_target_func;
char* break_target_num;
struct token_list* break_frame;
int current_count;
struct type* last_type;

/* Imported functions */
char* parse_string(char* string);
int escape_lookup(char* c);
char* numerate_number(int a);
void require_match(char* message, char* required);
void line_error();

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

struct token_list* expression(struct token_list* out, struct token_list* function);
struct token_list* function_call(struct token_list* out, struct token_list* function, char* s, int bool)
{
	require_match("ERROR in process_expression_list\nNo ( was found\n", "(");
	int passed = 0;
	out = emit("PUSH_edi\t# Prevent overwriting in recursion\n", out);
	out = emit("PUSH_ebp\t# Protect the old base pointer\n", out);
	out = emit("COPY_esp_to_edi\t# Copy new base pointer\n", out);

	if(global_token->s[0] != ')')
	{
		out = expression(out, function);
		out = emit("PUSH_eax\t#_process_expression1\n", out);
		passed = 1;

		while(global_token->s[0] == ',')
		{
			global_token = global_token->next;
			out = expression(out, function);
			out = emit("PUSH_eax\t#_process_expression2\n", out);
			passed = passed + 1;
		}
	}

	require_match("ERROR in process_expression_list\nNo ) was found\n", ")");

	if(TRUE == bool)
	{
		out = emit("LOAD_BASE_ADDRESS_eax %", out);
		out = emit(s, out);
		out = emit("\nLOAD_INTEGER\n", out);
		out = emit("COPY_edi_to_ebp\n", out);
		out = emit("CALL_eax\n", out);
	}
	else
	{
		out = emit("COPY_edi_to_ebp\n", out);
		out = emit("CALL_IMMEDIATE %FUNCTION_", out);
		out = emit(s, out);
		out = emit("\n", out);
	}

	for(; passed > 0; passed = passed - 1)
	{
		out = emit("POP_ebx\t# _process_expression_locals\n", out);
	}
	out = emit("POP_ebp\t# Restore old base pointer\n", out);
	out = emit("POP_edi\t# Prevent overwrite\n", out);
	return out;
}

struct token_list* constant_load(struct token_list* a, struct token_list* out)
{
	out = emit("LOAD_IMMEDIATE_eax %", out);
	out = emit(a->arguments->s, out);
	out = emit("\n", out);
	return out;
}

struct token_list* variable_load(struct token_list* a, struct token_list* out, struct token_list* function)
{
	if(match("FUNCTION", a->type->name) && match("(", global_token->s))
	{
		return function_call(out, function, numerate_number(a->depth), TRUE);
	}
	current_target = a->type;
	out = emit("LOAD_BASE_ADDRESS_eax %", out);
	out = emit(numerate_number(a->depth), out);
	out = emit("\n", out);
	if(!match("=", global_token->s) && !match("char**", a->type->name)) out = emit("LOAD_INTEGER\n", out);
	return out;
}

struct token_list* function_load(struct token_list* a, struct token_list* out, struct token_list* function)
{
	if(match("(", global_token->s)) return function_call(out, function, a->s, FALSE);

	out = emit("LOAD_IMMEDIATE_eax &FUNCTION_", out);
	out = emit(a->s, out);
	out = emit("\n", out);
	return out;
}

struct token_list* global_load(struct token_list* a, struct token_list* out)
{
	current_target = a->type;
	out = emit("LOAD_IMMEDIATE_eax &GLOBAL_", out);
	out = emit(a->s, out);
	out = emit("\n", out);
	if(!match("=", global_token->s)) out = emit("LOAD_INTEGER\n", out);
	return out;
}

/*
 * primary-expr:
 * FAILURE
 * "String"
 * 'Char'
 * [0-9]*
 * [a-z,A-Z]*
 * ( expression )
 */

struct token_list* primary_expr_failure()
{
	file_print("Recieved ", stderr);
	file_print(global_token->s, stderr);
	file_print(" in primary_expr\n", stderr);
	line_error();
	exit(EXIT_FAILURE);
}

struct token_list* uniqueID(char* s, struct token_list* out, char* num)
{
	out = emit(s, out);
	out = emit("_", out);
	out = emit(num, out);
	out = emit("\n", out);
	return out;
}

struct token_list* primary_expr_string(struct token_list* out, struct token_list* function)
{
	char* number_string = numerate_number(current_count);
	out = emit("LOAD_IMMEDIATE_eax &STRING_", out);
	out = uniqueID(function->s, out, number_string);

	/* The target */
	strings_list = emit(":STRING_", strings_list);
	strings_list = uniqueID(function->s, strings_list, number_string);

	/* Parse the string */
	strings_list = emit(parse_string(global_token->s), strings_list);
	global_token = global_token->next;

	current_count = current_count + 1;
	return out;
}

struct token_list* primary_expr_char(struct token_list* out)
{
	out = emit("LOAD_IMMEDIATE_eax %", out);
	if('\\' == global_token->s[1])
	{
		out = emit(numerate_number(escape_lookup(global_token->s + 1)), out);
	}
	else
	{
		out = emit(numerate_number(global_token->s[1]), out);
	}
	out = emit("\n", out);
	global_token = global_token->next;
	return out;
}

struct token_list* primary_expr_number(struct token_list* out)
{
	out = emit("LOAD_IMMEDIATE_eax %", out);
	out = emit(global_token->s, out);
	out = emit("\n", out);
	global_token = global_token->next;
	return out;
}

struct token_list* primary_expr_variable(struct token_list* out, struct token_list* function)
{
	char* s = global_token->s;
	global_token = global_token->next;
	struct token_list* a = sym_lookup(s, global_constant_list);
	if(NULL != a) return constant_load(a, out);

	a= sym_lookup(s, function->locals);
	if(NULL != a) return variable_load(a, out, function);

	a = sym_lookup(s, function->arguments);
	if(NULL != a) return variable_load(a, out, function);

	a= sym_lookup(s, global_function_list);
	if(NULL != a) return function_load(a, out, function);

	a = sym_lookup(s, global_symbol_list);
	if(NULL != a) return global_load(a, out);

	file_print(s ,stderr);
	file_print(" is not a defined symbol\n", stderr);
	line_error();
	exit(EXIT_FAILURE);
}

struct token_list* primary_expr(struct token_list* out, struct token_list* function);
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

struct token_list* common_recursion(struct token_list* out, struct token_list* function, FUNCTION f)
{
	last_type = current_target;
	global_token = global_token->next;
	out = emit("PUSH_eax\t#_common_recursion\n", out);
	out = f(out, function);
	current_target = promote_type(current_target, last_type);
	out = emit("POP_ebx\t# _common_recursion\n", out);
	return out;
}

struct token_list* general_recursion(struct token_list* out, struct token_list* function, FUNCTION f, char* s, char* name, FUNCTION iterate)
{
	if(match(name, global_token->s))
	{
		out = common_recursion(out, function, f);
		out = emit(s, out);
		out = iterate(out, function);
	}
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

struct token_list* postfix_expr_arrow(struct token_list* out)
{
	out = emit("# looking up offset\n", out);
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
		file_print(" does not exist\n", stderr);
		line_error();
		exit(EXIT_FAILURE);
	}
	if(0 != i->offset)
	{
		out = emit("# -> offset calculation\n", out);
		out = emit("LOAD_IMMEDIATE_ebx %", out);
		out = emit(numerate_number(i->offset), out);
		out = emit("\nADD_ebx_to_eax\n", out);
	}
	if(!match("=", global_token->next->s) && !match("char**",i->type->name))
	{
		out = emit("LOAD_INTEGER\n", out);
	}
	current_target = i->type;
	global_token = global_token->next;
	return out;
}

struct token_list* postfix_expr_array(struct token_list* out, struct token_list* function)
{
	struct type* array = current_target;
	out = common_recursion(out, function, expression);
	current_target = array;
	char* assign;

	/* Add support for Ints */
	if(!match("char*",  current_target->name))
	{
		out = emit("SAL_eax_Immediate8 !", out);
		out = emit(numerate_number(ceil_log2(current_target->indirect->size)), out);
		out = emit("\n", out);
		assign = "LOAD_INTEGER\n";
	}
	else
	{
		assign = "LOAD_BYTE\n";
	}

	out = emit("ADD_ebx_to_eax\n", out);
	require_match("ERROR in postfix_expr\nMissing ]\n", "]");

	if(match("=", global_token->s))
	{
		assign = "";
	}

	out = emit(assign, out);
	return out;
}

/*
 * unary-expr:
 *         postfix-expr
 *         - postfix-expr
 *         !postfix-expr
 *         sizeof ( type )
 */
struct type* type_name();
struct token_list* unary_expr_sizeof(struct token_list* out)
{
	global_token = global_token->next;
	require_match("ERROR in unary_expr\nMissing (\n", "(");
	struct type* a = type_name();
	require_match("ERROR in unary_expr\nMissing )\n", ")");

	out = emit("LOAD_IMMEDIATE_eax %", out);
	out = emit(numerate_number(a->size), out);
	out = emit("\n", out);
	return out;
}

struct token_list* postfix_expr(struct token_list* out, struct token_list* function);
struct token_list* unary_expr_not(struct token_list* out, struct token_list* function)
{
	out = emit("LOAD_IMMEDIATE_eax %1\n", out);
	out = common_recursion(out, function, postfix_expr);
	out = emit("XOR_ebx_eax_into_eax\n", out);
	return out;
}

struct token_list* unary_expr_negation(struct token_list* out, struct token_list* function)
{
	out = emit("LOAD_IMMEDIATE_eax %0\n", out);
	out = common_recursion(out, function, primary_expr);
	out = emit("SUBTRACT_eax_from_ebx_into_ebx\nMOVE_ebx_to_eax\n", out);
	return out;
}

struct token_list* postfix_expr_stub(struct token_list* out, struct token_list* function)
{
	if(match("[", global_token->s))
	{
		out = postfix_expr_array(out, function);
		out = postfix_expr_stub(out, function);
	}

	if(match("->", global_token->s))
	{
		out = postfix_expr_arrow(out);
		out = postfix_expr_stub(out, function);
	}

	return out;
}

struct token_list* postfix_expr(struct token_list* out, struct token_list* function)
{
	out = primary_expr(out, function);
	out = postfix_expr_stub(out, function);
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
 *         additive-expr << postfix-expr
 *         additive-expr >> postfix-expr
 */
struct token_list* additive_expr_stub(struct token_list* out, struct token_list* function)
{
	out = general_recursion(out, function, postfix_expr, "ADD_ebx_to_eax\n", "+", additive_expr_stub);
	out = general_recursion(out, function, postfix_expr, "SUBTRACT_eax_from_ebx_into_ebx\nMOVE_ebx_to_eax\n", "-", additive_expr_stub);
	out = general_recursion(out, function, postfix_expr, "MULTIPLY_eax_by_ebx_into_eax\n", "*", additive_expr_stub);
	out = general_recursion(out, function, postfix_expr, "XCHG_eax_ebx\nLOAD_IMMEDIATE_edx %0\nDIVIDE_eax_by_ebx_into_eax\n", "/", additive_expr_stub);
	out = general_recursion(out, function, postfix_expr, "XCHG_eax_ebx\nLOAD_IMMEDIATE_edx %0\nMODULUS_eax_from_ebx_into_ebx\nMOVE_edx_to_eax\n", "%", additive_expr_stub);
	out = general_recursion(out, function, postfix_expr, "COPY_eax_to_ecx\nCOPY_ebx_to_eax\nSAL_eax_cl\n", "<<", additive_expr_stub);
	out = general_recursion(out, function, postfix_expr, "COPY_eax_to_ecx\nCOPY_ebx_to_eax\nSAR_eax_cl\n", ">>", additive_expr_stub);

	return out;
}


struct token_list* additive_expr(struct token_list* out, struct token_list* function)
{
	out = postfix_expr(out, function);
	out = additive_expr_stub(out, function);
	return out;
}


/*
 * relational-expr:
 *         additive_expr
 *         relational-expr < additive_expr
 *         relational-expr <= additive_expr
 *         relational-expr >= additive_expr
 *         relational-expr > additive_expr
 */

struct token_list* relational_expr_stub(struct token_list* out, struct token_list* function)
{
	out = general_recursion(out, function, additive_expr, "CMP\nSETL\nMOVEZBL\n", "<", relational_expr_stub);
	out = general_recursion(out, function, additive_expr, "CMP\nSETLE\nMOVEZBL\n", "<=", relational_expr_stub);
	out = general_recursion(out, function, additive_expr, "CMP\nSETGE\nMOVEZBL\n", ">=", relational_expr_stub);
	out = general_recursion(out, function, additive_expr, "CMP\nSETG\nMOVEZBL\n", ">", relational_expr_stub);
	out = general_recursion(out, function, additive_expr, "CMP\nSETE\nMOVEZBL\n", "==", relational_expr_stub);
	out = general_recursion(out, function, additive_expr, "CMP\nSETNE\nMOVEZBL\n", "!=", relational_expr_stub);
	return out;
}

struct token_list* relational_expr(struct token_list* out, struct token_list* function)
{
	out = additive_expr(out, function);
	out = relational_expr_stub(out, function);
	return out;
}

/*
 * bitwise-expr:
 *         relational-expr
 *         bitwise-expr & bitwise-expr
 *         bitwise-expr && bitwise-expr
 *         bitwise-expr | bitwise-expr
 *         bitwise-expr || bitwise-expr
 *         bitwise-expr ^ bitwise-expr
 */
struct token_list* bitwise_expr_stub(struct token_list* out, struct token_list* function)
{
	out = general_recursion(out, function, relational_expr, "AND_eax_ebx\n", "&", bitwise_expr_stub);
	out = general_recursion(out, function, relational_expr, "AND_eax_ebx\n", "&&", bitwise_expr_stub);
	out = general_recursion(out, function, relational_expr, "OR_eax_ebx\n", "|", bitwise_expr_stub);
	out = general_recursion(out, function, relational_expr, "OR_eax_ebx\n", "||", bitwise_expr_stub);
	out = general_recursion(out, function, relational_expr, "XOR_ebx_eax_into_eax\n", "^", bitwise_expr_stub);
	return out;
}


struct token_list* bitwise_expr(struct token_list* out, struct token_list* function)
{
	out = relational_expr(out, function);
	out = bitwise_expr_stub(out, function);
	if(match("=", global_token->s))
	{
		char* store;
		if(match("]", global_token->prev->s) && match("char*", current_target->name))
		{
			store = "STORE_CHAR\n";
		}
		else
		{
			store = "STORE_INTEGER\n";
		}

		out = common_recursion(out, function, expression);
		out = emit(store, out);
	}
	return out;
}

/*
 * expression:
 *         bitwise-or-expr
 *         bitwise-or-expr = expression
 */

struct token_list* primary_expr(struct token_list* out, struct token_list* function)
{
	if(match("-", global_token->s)) out = unary_expr_negation(out, function);
	else if(match("!", global_token->s)) out = unary_expr_not(out, function);
	else if(match("sizeof", global_token->s)) out = unary_expr_sizeof(out);
	else if(global_token->s[0] == '(')
	{
		global_token = global_token->next;
		out = expression(out, function);
		require_match("Error in Primary expression\nDidn't get )\n", ")");
	}
	else if((('a' <= global_token->s[0]) && (global_token->s[0] <= 'z')) || (('A' <= global_token->s[0]) && (global_token->s[0] <= 'Z'))) out = primary_expr_variable(out, function);
	else if(('0' <= global_token->s[0]) && (global_token->s[0] <= '9')) out = primary_expr_number(out);
	else if(global_token->s[0] == '\'') out = primary_expr_char(out);
	else if(global_token->s[0] == '"') out = primary_expr_string(out, function);
	else primary_expr_failure();
	return out;
}

struct token_list* expression(struct token_list* out, struct token_list* function)
{
	out = bitwise_expr(out, function);
	return out;
}


/* Process local variable */
struct token_list* collect_local(struct token_list* out, struct token_list* function)
{
	struct type* type_size = type_name();
	out = emit("# Defining local ", out);
	out = emit(global_token->s, out);
	out = emit("\n", out);

	struct token_list* a = sym_declare(global_token->s, type_size, function->locals);
	if(match("main", function->s) && (NULL == function->locals))
	{
		a->depth = -4;
	}
	else if((NULL == function->arguments) && (NULL == function->locals))
	{
		a->depth = -8;
	}
	else if(NULL == function->locals)
	{
		a->depth = function->arguments->depth - 8;
	}
	else
	{
		a->depth = function->locals->depth - 4;
	}

	function->locals = a;
	global_token = global_token->next;

	if(match("=", global_token->s))
	{
		global_token = global_token->next;
		out = expression(out, function);
	}

	require_match("ERROR in collect_local\nMissing ;\n", ";");

	out = emit("PUSH_eax\t#", out);
	out = emit(a->s, out);
	out = emit("\n", out);
	return out;
}

struct token_list* statement(struct token_list* out, struct token_list* function);

/* Evaluate if statements */
struct token_list* process_if(struct token_list* out, struct token_list* function)
{
	char* number_string = numerate_number(current_count);
	current_count = current_count + 1;

	out = emit("# IF_", out);
	out = uniqueID(function->s, out, number_string);

	global_token = global_token->next;
	require_match("ERROR in process_if\nMISSING (\n", "(");
	out = expression(out, function);

	out = emit("TEST\nJUMP_EQ %ELSE_", out);
	out = uniqueID(function->s, out, number_string);

	require_match("ERROR in process_if\nMISSING )\n", ")");
	out = statement(out, function);

	out = emit("JUMP %_END_IF_", out);
	out = uniqueID(function->s, out, number_string);
	out = emit(":ELSE_", out);
	out = uniqueID(function->s, out, number_string);

	if(match("else", global_token->s))
	{
		global_token = global_token->next;
		out = statement(out, function);
	}
	out = emit(":_END_IF_", out);
	out = uniqueID(function->s, out, number_string);
	return out;
}

struct token_list* process_for(struct token_list* out, struct token_list* function)
{
	char* number_string = numerate_number(current_count);
	current_count = current_count + 1;

	char* nested_break_head = break_target_head;
	char* nested_break_func = break_target_func;
	char* nested_break_num = break_target_num;
	struct token_list* nested_locals = break_frame;
	break_frame = function->locals;
	break_target_head = "FOR_END_";
	break_target_func = function->s;
	break_target_num = number_string;

	out = emit("# FOR_initialization_", out);
	out = uniqueID(function->s, out, number_string);

	global_token = global_token->next;

	require_match("ERROR in process_for\nMISSING (\n", "(");
	if(!match(";",global_token->s))
	{
		out = expression(out, function);
	}

	out = emit(":FOR_", out);
	out = uniqueID(function->s, out, number_string);

	require_match("ERROR in process_for\nMISSING ;1\n", ";");
	out = expression(out, function);

	out = emit("TEST\nJUMP_EQ %FOR_END_", out);
	out = uniqueID(function->s, out, number_string);
	out = emit("JUMP %FOR_THEN_", out);
	out = uniqueID(function->s, out, number_string);
	out = emit(":FOR_ITER_", out);
	out = uniqueID(function->s, out, number_string);

	require_match("ERROR in process_for\nMISSING ;2\n", ";");
	out = expression(out, function);

	out = emit("JUMP %FOR_", out);
	out = uniqueID(function->s, out, number_string);
	out = emit(":FOR_THEN_", out);
	out = uniqueID(function->s, out, number_string);

	require_match("ERROR in process_for\nMISSING )\n", ")");
	out = statement(out, function);

	out = emit("JUMP %FOR_ITER_", out);
	out = uniqueID(function->s, out, number_string);
	out = emit(":FOR_END_", out);
	out = uniqueID(function->s, out, number_string);

	break_target_head = nested_break_head;
	break_target_func = nested_break_func;
	break_target_num = nested_break_num;
	break_frame = nested_locals;
	return out;
}

/* Process Assembly statements */
struct token_list* process_asm(struct token_list* out)
{
	global_token = global_token->next;
	require_match("ERROR in process_asm\nMISSING (\n", "(");
	while(34 == global_token->s[0])
	{/* 34 == " */
		out = emit((global_token->s + 1), out);
		out = emit("\n", out);
		global_token = global_token->next;
	}
	require_match("ERROR in process_asm\nMISSING )\n", ")");
	require_match("ERROR in process_asm\nMISSING ;\n", ";");
	return out;
}

/* Process do while loops */
struct token_list* process_do(struct token_list* out, struct token_list* function)
{
	char* number_string = numerate_number(current_count);
	current_count = current_count + 1;

	char* nested_break_head = break_target_head;
	char* nested_break_func = break_target_func;
	char* nested_break_num = break_target_num;
	struct token_list* nested_locals = break_frame;
	break_frame = function->locals;
	break_target_head = "DO_END_";
	break_target_func = function->s;
	break_target_num = number_string;

	out = emit(":DO_", out);
	out = uniqueID(function->s, out, number_string);

	global_token = global_token->next;
	out = statement(out, function);

	require_match("ERROR in process_do\nMISSING while\n", "while");
	require_match("ERROR in process_do\nMISSING (\n", "(");
	out = expression(out, function);
	require_match("ERROR in process_do\nMISSING )\n", ")");
	require_match("ERROR in process_do\nMISSING ;\n", ";");

	out = emit("TEST\nJUMP_NE %DO_", out);
	out = uniqueID(function->s, out, number_string);
	out = emit(":DO_END_", out);
	out = uniqueID(function->s, out, number_string);

	break_frame = nested_locals;
	break_target_head = nested_break_head;
	break_target_func = nested_break_func;
	break_target_num = nested_break_num;
	return out;
}


/* Process while loops */
struct token_list* process_while(struct token_list* out, struct token_list* function)
{
	char* number_string = numerate_number(current_count);
	current_count = current_count + 1;

	char* nested_break_head = break_target_head;
	char* nested_break_func = break_target_func;
	char* nested_break_num = break_target_num;
	struct token_list* nested_locals = break_frame;
	break_frame = function->locals;

	break_target_head = "END_WHILE_";
	break_target_func = function->s;
	break_target_num = number_string;

	out = emit(":WHILE_", out);
	out = uniqueID(function->s, out, number_string);

	global_token = global_token->next;
	require_match("ERROR in process_while\nMISSING (\n", "(");
	out = expression(out, function);

	out = emit("TEST\nJUMP_EQ %END_WHILE_", out);
	out = uniqueID(function->s, out, number_string);
	out = emit("# THEN_while_", out);
	out = uniqueID(function->s, out, number_string);

	require_match("ERROR in process_while\nMISSING )\n", ")");
	out = statement(out, function);

	out = emit("JUMP %WHILE_", out);
	out = uniqueID(function->s, out, number_string);
	out = emit(":END_WHILE_", out);
	out = uniqueID(function->s, out, number_string);

	break_frame = nested_locals;
	break_target_head = nested_break_head;
	break_target_func = nested_break_func;
	break_target_num = nested_break_num;
	return out;
}

/* Ensure that functions return */
struct token_list* return_result(struct token_list* out, struct token_list* function)
{
	global_token = global_token->next;
	if(global_token->s[0] != ';') out = expression(out, function);

	require_match("ERROR in return_result\nMISSING ;\n", ";");

	struct token_list* i;
	for(i = function->locals; NULL != i; i = i->next)
	{
		out = emit("POP_ebx\t# _return_result_locals\n", out);
	}
	out = emit("RETURN\n", out);
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
	struct token_list* i;
	for(i = function->locals; frame != i; i = i->next)
	{
		if(NULL == function->locals) return out;
		if(!match("RETURN\n", out->s))
		{
			out = emit( "POP_ebx\t# _recursive_statement_locals\n", out);
		}
		function->locals = function->locals->next;
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
		out = emit("\t#C goto label\n", out);
		global_token = global_token->next;
	}
	else if(((NULL == sym_lookup(global_token->s, function->locals)) &&
	         (NULL == sym_lookup(global_token->s, function->arguments)) &&
	         (NULL != lookup_type(global_token->s))) ||
	          match("struct", global_token->s))
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
		out = emit("JUMP %", out);
		out = emit(global_token->s, out);
		out = emit("\n", out);
		global_token = global_token->next;
		require_match("ERROR in statement\nMissing ;\n", ";");
	}
	else if(match("return", global_token->s))
	{
		out = return_result(out, function);
	}
	else if(match("break", global_token->s))
	{
		if(NULL == break_target_head)
		{
			file_print("Not inside of a loop or case statement", stderr);
			line_error();
			exit(EXIT_FAILURE);
		}
		struct token_list* i = function->locals;
		while(i != break_frame)
		{
			if(NULL == i) break;
			out = emit("POP_ebx\t# break_cleanup_locals\n", out);
			i = i->next;
		}
		global_token = global_token->next;
		out = emit("JUMP %", out);
		out = emit(break_target_head, out);
		out = emit(break_target_func, out);
		out = emit("_", out);
		out = emit(break_target_num, out);
		out = emit("\n", out);
		require_match("ERROR in statement\nMissing ;\n", ";");
	}
	else if(match("continue", global_token->s))
	{
		global_token = global_token->next;
		out = emit("\n#continue statement\n",out);
		require_match("ERROR in statement\nMissing ;\n", ";");
	}
	else
	{
		out = expression(out, function);
		require_match("ERROR in statement\nMISSING ;\n", ";");
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
			if(match("main", function->s))
			{
				if(match("argc", a->s)) a->depth = 4;
				if(match("argv", a->s)) a->depth = 8;
			}
			else if(NULL == function->arguments)
			{
				a->depth = -4;
			}
			else
			{
				a->depth = function->arguments->depth - 4;
			}

			function->arguments = a;
		}

		/* foo(int,char,void) doesn't need anything done */
		global_token = global_token->next;

		/* ignore trailing comma (needed for foo(bar(), 1); expressions*/
		if(global_token->s[0] == ',') global_token = global_token->next;
	}
	global_token = global_token->next;
}

struct token_list* declare_function(struct token_list* out)
{
	current_count = 0;
	struct token_list* func = sym_declare(global_token->prev->s, calloc(1, sizeof(struct type)), global_function_list);
	collect_arguments(func);

	/* allow previously defined functions to be looked up */
	global_function_list = func;

	/* If just a prototype don't waste time */
	if(global_token->s[0] == ';') global_token = global_token->next;
	else
	{
		out = emit("# Defining function ", out);
		out = emit(func->s, out);
		out = emit("\n", out);
		out = emit(":FUNCTION_", out);
		out = emit(func->s, out);
		out = emit("\n", out);
		if(match("main", func->s))
		{
			out = emit("COPY_esp_to_ebp\t# Deal with special case\n", out);
		}
		out = statement(out, func);

		/* Prevent duplicate RETURNS */
		if(!match("RETURN\n", out->s))
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
				globals_list = emit(":GLOBAL_", globals_list);
				globals_list = emit(global_token->prev->s, globals_list);
				globals_list = emit("\nNOP\n", globals_list);

				global_token = global_token->next;
			}
			else if(match("=",global_token->s))
			{
				/* Add to global symbol table */
				global_symbol_list = sym_declare(global_token->prev->s, type_size, global_symbol_list);

				/* Store the global's value*/
				globals_list = emit(":GLOBAL_", globals_list);
				globals_list = emit(global_token->prev->s, globals_list);
				globals_list = emit("\n", globals_list);
				global_token = global_token->next;
				if(('0' <= global_token->s[0]) & (global_token->s[0] <= '9'))
				{ /* Assume Int */
					globals_list = emit("%", globals_list);
					globals_list = emit(global_token->s, globals_list);
					globals_list = emit("\n", globals_list);
				}
				else if(('"' == global_token->s[0]))
				{ /* Assume a string*/
					globals_list = emit(parse_string(global_token->s), globals_list);
				}
				else
				{
					file_print("Recieved ", stderr);
					file_print(global_token->s, stderr);
					file_print(" in program\n", stderr);
					line_error();
					exit(EXIT_FAILURE);
				}

				global_token = global_token->next;
				require_match("ERROR in Program\nMissing ;\n", ";");
			}
			else if(global_token->s[0] == '(') out = declare_function(out);
			else
			{
				file_print("Recieved ", stderr);
				file_print(global_token->s, stderr);
				file_print(" in program\n", stderr);
				line_error();
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
