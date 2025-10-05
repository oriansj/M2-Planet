/* Copyright (C) 2016 Jeremiah Orians
 * Copyright (C) 2018 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
 * Copyright (C) 2020 deesix <deesix@tuta.io>
 * Copyright (C) 2021 Andrius Štikonas <andrius@stikonas.eu>
 * Copyright (C) 2025 Gtker
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

#include "cc_emit.h"
#include "gcc_req.h"
#include <stdint.h>

/* Global lists */
struct token_list* global_symbol_list;
struct token_list* global_function_list;

/* Core lists for this file */
struct token_list* function;

/* What we are currently working on */
struct type* current_target;
char* break_target_head;
char* break_target_func;
char* break_target_num;
char* continue_target_head;
struct token_list* break_frame;
int current_count;
int Address_of;

/* Imported functions */
char* int2str(int x, int base, int signed_p);
int strtoint(char *a);
char* parse_string(char* string);
int escape_lookup(char* c);
void require(int bool, char* error);
struct token_list* reverse_list(struct token_list* head);
struct type *mirror_type(struct type *source);
struct type* new_function_pointer_typedef(char* name);
struct type* add_primitive(struct type* a);

int global_static_array(struct type*, char*);
void declare_global_variable(struct type* type_size, struct token_list* variable);

struct type* fallible_type_name(void);
struct type* type_name(void);

char* parse_function_pointer(void);

int type_is_pointer(struct type* type_size)
{
	return type_size->type != type_size || (type_size->options & TO_FUNCTION_POINTER);
}

int type_is_struct_or_union(struct type* type_size)
{
	return type_size->members != NULL;
}

char* create_unique_id(char* prefix, char* s, char* num)
{
	return concat_strings4(prefix, s, "_", num);
}

struct token_list* sym_declare(char *s, struct type* t, struct token_list* list, int options)
{
	struct token_list* a = calloc(1, sizeof(struct token_list));
	require(NULL != a, "Exhausted memory while attempting to declare a symbol\n");
	a->next = list;
	a->s = s;
	a->type = t;
	a->array_modifier = 1;
	a->options = options;
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

struct token_list* static_variable_lookup(char* s)
{
	struct static_variable_list* statics = function_static_variables_list;
	while(statics != NULL)
	{
		if(match(s, statics->local_variable_name))
		{
			return statics->global_variable;
		}
		statics = statics->next;
	}

	return NULL;
}

void line_error_token(struct token_list *token)
{
	if(NULL == token)
	{
		fputs("EOF reached inside of line_error\n", stderr);
		fputs("problem at end of file\n", stderr);
		return;
	}
	fputs(token->filename, stderr);
	fputs(":", stderr);
	fputs(int2str(token->linenumber, 10, TRUE), stderr);
	fputs(":", stderr);
}

void line_error(void)
{
	line_error_token(global_token);
}

/* Checks if current global_token is NULL and exits if it is. */
void require_token(void)
{
	if(NULL == global_token)
	{
		line_error_token(global_token);
		exit(EXIT_FAILURE);
	}
}

/* Advances token and checks for NULL. */
void require_extra_token(void)
{
	global_token = global_token->next;
	require_token();
}

void require_match(char* message, char* required)
{
	if(NULL == global_token)
	{
		line_error();
		fputs("EOF reached inside of require match\n", stderr);
		fputs("problem at end of file\n", stderr);
		fputs(message, stderr);
		exit(EXIT_FAILURE);
	}
	if(!match(global_token->s, required))
	{
		line_error();
		fputs(message, stderr);
		exit(EXIT_FAILURE);
	}
	global_token = global_token->next;
}

void maybe_bootstrap_error(char* feature)
{
	if (BOOTSTRAP_MODE)
	{
		line_error();
		fputs(feature, stderr);
		fputs(" is not supported in --bootstrap-mode\n", stderr);
		exit(EXIT_FAILURE);
	}
}

int unary_expr_sizeof(void);
int constant_unary_expression(void)
{
	if('-' == global_token->s[0])
	{
		require_extra_token();
		return -constant_unary_expression();
	}
	else if('+' == global_token->s[0])
	{
		require_extra_token();
		return constant_unary_expression();
	}
	else if(match("sizeof", global_token->s))
	{
		return unary_expr_sizeof();
	}
	else if(global_token->s[0] == '\'')
	{
		int val = escape_lookup(global_token->s + 1);
		global_token = global_token->next;
		return val;
	}
	else if(in_set(global_token->s[0], "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"))
	{
		struct token_list* lookup = sym_lookup(global_token->s, global_constant_list);
		if(lookup != NULL)
		{
			require_extra_token();
			return strtoint(lookup->arguments->s);
		}
		else
		{
			line_error();
			fputs("Unable to find symbol '", stderr);
			fputs(global_token->s, stderr);
			fputs("' for use in constant expression.\n", stderr);
			exit(EXIT_FAILURE);
		}
	}
	else if(in_set(global_token->s[0], "0123456789"))
	{
		require_extra_token();
		return strtoint(global_token->prev->s);
	}

	line_error();
	fputs("Invalid token '", stderr);
	fputs(global_token->s, stderr);
	fputs("' used in constant_expression_term.\n", stderr);
	exit(EXIT_FAILURE);
}

/* global_token should start on the first part of the expression
 * and it will end one token past the end of the expression. */
int constant_expression(void)
{
	/* We go from the highest precedence operators down to the lowest */
	int lhs = constant_unary_expression();

	if(global_token->s[0] == '+')
	{
		require_extra_token();
		return lhs + constant_expression();
	}
	else if(global_token->s[0] == '-')
	{
		require_extra_token();
		return lhs - constant_expression();
	}
	else if(global_token->s[0] == ',' || global_token->s[0] == ']' || global_token->s[0] == ';' || global_token->s[0] == '}' || global_token->s[0] == ':')
	{
		return lhs;
	}

	line_error();
	fputs("Invalid token '", stderr);
	fputs(global_token->s, stderr);
	fputs("' used in constant expression.\n", stderr);
	exit(EXIT_FAILURE);
}

void expression(void);
void function_call(struct token_list* s, int is_function_pointer)
{
	require_match("ERROR in process_expression_list\nNo ( was found\n", "(");
	require(NULL != global_token, "Improper function call\n");

	emit_push(REGISTER_TEMP, "Protect temp register we are going to use");
	if((AARCH64 == Architecture) || (RISCV64 == Architecture) || (RISCV32 == Architecture))
	{
		emit_push(REGISTER_RETURN, "Protect the old return pointer (link)");
	}
	emit_push(REGISTER_BASE, "Protect the old base pointer");
	emit_push(REGISTER_LOCALS, "Protect the old locals pointer");

	emit_move(REGISTER_TEMP, REGISTER_STACK, "Copy new base pointer");

	if(is_function_pointer)
	{
		emit_move(REGISTER_TEMP2, REGISTER_ZERO, "Save function pointer address");
	}

	int passed = 0;
	while(global_token->s[0] != ')')
	{
		expression();
		require(NULL != global_token, "incomplete function call, received EOF instead of )\n");
		emit_push(REGISTER_ZERO, "function argument");
		passed = passed + 1;

		if(global_token->s[0] == ',')
		{
			require_extra_token();
		}
	}

	require_match("ERROR in process_expression_list\nNo ) was found\n", ")");

	if(ARMV7L == Architecture)
	{
		emit_push(REGISTER_RETURN, "Protect the old link register");
	}

	emit_move(REGISTER_BASE, REGISTER_TEMP, "Set new base pointer");

	if(TRUE == is_function_pointer)
	{
		emit_move(REGISTER_ZERO, REGISTER_TEMP2, "Restore function pointer");

		if(Architecture & ARCH_FAMILY_KNIGHT)
		{
			emit_out("CALL R0 R15\n");
		}
		else if(X86 == Architecture)
		{
			emit_out("call_eax\n");
		}
		else if(AMD64 == Architecture)
		{
			emit_out("call_rax\n");
		}
		else if(ARMV7L == Architecture)
		{
			emit_out("'3' R0 CALL_REG_ALWAYS\n");
			emit_pop(REGISTER_RETURN, "Prevent overwrite");
		}
		else if(AARCH64 == Architecture)
		{
			emit_move(REGISTER_TEMP, REGISTER_ZERO, "function pointer call");
			emit_out("BLR_X16\n");
		}
		else if(Architecture & ARCH_FAMILY_RISCV)
		{
			emit_out("rd_ra rs1_a0 jalr\n");
		}
	}
	else
	{
		if(Architecture & ARCH_FAMILY_KNIGHT)
		{
			emit_load_named_immediate(REGISTER_ZERO, "FUNCTION_", s->s, "function call");
			emit_out("CALL R0 R15\n");
		}
		else if(Architecture & ARCH_FAMILY_X86)
		{
			emit_out("call %FUNCTION_");
			emit_out(s->s);
			emit_out("\n");
		}
		else if(ARMV7L == Architecture)
		{
			emit_out("^~FUNCTION_");
			emit_out(s->s);
			emit_out(" CALL_ALWAYS\n");
			emit_pop(REGISTER_RETURN, "Restore the old link register");
		}
		else if(AARCH64 == Architecture)
		{
			emit_load_named_immediate(REGISTER_TEMP, "FUNCTION_", s->s, "function call");
			emit_out("BLR_X16\n");
		}
		else if(Architecture & ARCH_FAMILY_RISCV)
		{
			emit_out("rd_ra $FUNCTION_");
			emit_out(s->s);
			emit_out(" jal\n");
		}
	}

	if(passed > 0)
	{
		emit_move(REGISTER_STACK, REGISTER_BASE, "Clean up function arguments");
	}

	emit_pop(REGISTER_LOCALS, "Restore old locals pointer");
	emit_pop(REGISTER_BASE, "Restore old base pointer");
	if((AARCH64 == Architecture) || (RISCV64 == Architecture) || (RISCV32 == Architecture))
	{
		emit_pop(REGISTER_RETURN, "Restore old return pointer (link)");
	}
	emit_pop(REGISTER_TEMP, "Restore temp register");
}

char* load_value_signed(unsigned size)
{
	if(size == 1)
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) return "LOAD8 R0 R0 0\n";
		else if(X86 == Architecture) return "movsx_eax,BYTE_PTR_[eax]\n";
		else if(AMD64 == Architecture) return "movsx_rax,BYTE_PTR_[rax]\n";
		else if(ARMV7L == Architecture) return "LOADS8 R0 LOAD R0 HALF_MEMORY\n";
		else if(AARCH64 == Architecture) return "LDRSB_X0_[X0]\n";
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) return "rd_a0 rs1_a0 lb\n";
	}
	else if(size == 2)
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) return "LOAD16 R0 R0 0\n";
		else if(X86 == Architecture) return "movsx_eax,WORD_PTR_[eax]\n";
		else if(AMD64 == Architecture) return "movsx_rax,WORD_PTR_[rax]\n";
		else if(ARMV7L == Architecture) return "LOADS16 R0 LOAD R0 HALF_MEMORY\n";
		else if(AARCH64 == Architecture) return "LDRSH_X0_[X0]\n";
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) return "rd_a0 rs1_a0 lh\n";
	}
	else if(size == 4)
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) return "LOAD R0 R0 0\n";
		else if(X86 == Architecture) return "mov_eax,[eax]\n";
		else if(AMD64 == Architecture) return "movsx_rax,DWORD_PTR_[rax]\n";
		else if(ARMV7L == Architecture) return "!0 R0 LOAD32 R0 MEMORY\n";
		else if(AARCH64 == Architecture) return "LDR_W0_[X0]\n";
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) return "rd_a0 rs1_a0 lw\n";
	}
	else if(size == 8)
	{
		if(AMD64 == Architecture) return "mov_rax,[rax]\n";
		else if(AARCH64 == Architecture) return "DEREF_X0\n";
		else if(RISCV64 == Architecture) return "rd_a0 rs1_a0 ld\n";
	}
	line_error();
	fputs(" Got unsupported size ", stderr);
	fputs(int2str(size, 10, TRUE), stderr);
	fputs(" when trying to load value.\n", stderr);
	exit(EXIT_FAILURE);
}

char* load_value_unsigned(unsigned size)
{
	if(size == 1)
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) return "LOADU8 R0 R0 0\n";
		else if(X86 == Architecture) return "movzx_eax,BYTE_PTR_[eax]\n";
		else if(AMD64 == Architecture) return "movzx_rax,BYTE_PTR_[rax]\n";
		else if(ARMV7L == Architecture) return "!0 R0 LOAD R0 MEMORY\n";
		else if(AARCH64 == Architecture) return "DEREF_X0_BYTE\n";
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) return "rd_a0 rs1_a0 lbu\n";
	}
	else if(size == 2)
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) return "LOADU16 R0 R0 0\n";
		else if(X86 == Architecture) return "movzx_eax,WORD_PTR_[eax]\n";
		else if(AMD64 == Architecture) return "movzx_rax,WORD_PTR_[rax]\n";
		else if(ARMV7L == Architecture) return "NO_OFFSET R0 LOAD R0 HALF_MEMORY\n";
		else if(AARCH64 == Architecture) return "LDRH_W0_[X0]\n";
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) return "rd_a0 rs1_a0 lhu\n";
	}
	else if(size == 4)
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) return "LOAD R0 R0 0\n";
		else if(X86 == Architecture) return "mov_eax,[eax]\n";
		else if(AMD64 == Architecture) return "mov_eax,[rax]\n";
		else if(ARMV7L == Architecture) return "!0 R0 LOAD32 R0 MEMORY\n";
		else if(AARCH64 == Architecture) return "LDR_W0_[X0]\n";
		else if(RISCV32 == Architecture) return "rd_a0 rs1_a0 lw\n";
		else if(RISCV64 == Architecture) return "rd_a0 rs1_a0 lwu\n";
	}
	else if(size == 8)
	{
		if(AMD64 == Architecture) return "mov_rax,[rax]\n";
		else if(AARCH64 == Architecture) return "DEREF_X0\n";
		else if(RISCV64 == Architecture) return "rd_a0 rs1_a0 ld\n";
	}
	line_error();
	fputs(" Got unsupported size ", stderr);
	fputs(int2str(size, 10, TRUE), stderr);
	fputs(" when trying to load value.\n", stderr);
	exit(EXIT_FAILURE);
}

char* load_value(unsigned size, int is_signed)
{
	if(is_signed) return load_value_signed(size);
	return load_value_unsigned(size);
}

char* store_value(unsigned size)
{
	if(size == 1)
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) return "STORE8 R0 R1 0\n";
		else if(X86 == Architecture) return "mov_[ebx],al\n";
		else if(AMD64 == Architecture) return "mov_[rbx],al\n";
		else if(ARMV7L == Architecture) return "!0 R0 STORE8 R1 MEMORY\n";
		else if(AARCH64 == Architecture) return "STR_BYTE_W0_[X1]\n";
		else if(RISCV32 == Architecture || RISCV64 == Architecture) return "rs1_a1 rs2_a0 sb\n";
	}
	else if(size == 2)
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) return "STORE16 R0 R1 0\n";
		else if(X86 == Architecture) return "mov_[ebx],ax\n";
		else if(AMD64 == Architecture) return "mov_[rbx],ax\n";
		else if(ARMV7L == Architecture) return "NO_OFFSET R0 STORE16 R1 HALF_MEMORY\n";
		else if(AARCH64 == Architecture) return "STRH_W0_[X1]\n";
		else if(RISCV32 == Architecture || RISCV64 == Architecture) return "rs1_a1 rs2_a0 sh\n";
	}
	else if(size == 4)
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) return "STORE R0 R1 0\n";
		else if(X86 == Architecture) return "mov_[ebx],eax\n";
		else if(AMD64 == Architecture) return "mov_[rbx],eax\n";
		else if(ARMV7L == Architecture) return "!0 R0 STORE32 R1 MEMORY\n";
		else if(AARCH64 == Architecture) return "STR_W0_[X1]\n";
		else if(RISCV32 == Architecture || RISCV64 == Architecture) return "rs1_a1 rs2_a0 sw\n";
	}
	else if(size == 8)
	{
		if(AMD64 == Architecture) return "mov_[rbx],rax\n";
		else if(AARCH64 == Architecture) return "STR_X0_[X1]\n";
		else if(RISCV64 == Architecture) return "rs1_a1 rs2_a0 sd\n";
	}
	/* Should not happen but print error message. */
	fputs("Got unsupported size ", stderr);
	fputs(int2str(size, 10, TRUE), stderr);
	fputs(" when storing number in register.\n", stderr);
	line_error();
	exit(EXIT_FAILURE);
}

int is_compound_assignment(char* token)
{
	if(match("+=", token)) return TRUE;
	else if(match("-=", token)) return TRUE;
	else if(match("*=", token)) return TRUE;
	else if(match("/=", token)) return TRUE;
	else if(match("%=", token)) return TRUE;
	else if(match("<<=", token)) return TRUE;
	else if(match(">>=", token)) return TRUE;
	else if(match("&=", token)) return TRUE;
	else if(match("^=", token)) return TRUE;
	else if(match("|=", token)) return TRUE;
	return FALSE;
}

void postfix_expr_stub(void);

/*
 * primary-expr:
 * FAILURE
 * "String"
 * 'Char'
 * [0-9]*
 * [a-z,A-Z]*
 * ( expression )
 */

void primary_expr_failure(void)
{
	require(NULL != global_token, "hit EOF when expecting primary expression\n");
	line_error();
	fputs("Received ", stderr);
	fputs(global_token->s, stderr);
	fputs(" in primary_expr\n", stderr);
	exit(EXIT_FAILURE);
}

void primary_expr_string(void)
{
	char* number_string = int2str(current_count, 10, TRUE);
	current_count = current_count + 1;
	char* unique_id = create_unique_id("STRING_", function->s, number_string);

	emit_load_named_immediate(REGISTER_ZERO, "", unique_id, "primary expr string");

	/* The target */
	strings_list = emit("\n", emit(unique_id, emit(":", strings_list)));

	/* catch case of just "foo" from segfaulting */
	require(NULL != global_token->next, "a string by itself is not valid C\n");

	/* Parse the string */
	if('"' != global_token->next->s[0])
	{
		strings_list = emit(parse_string(global_token->s), strings_list);
		require_extra_token();
	}
	else
	{
		char* s = calloc(MAX_STRING, sizeof(char));

		/* prefix leading string */
		s[0] = '"';
		int i = 1;

		int used_string_concatenation = FALSE;
		int j;
		while('"' == global_token->s[0])
		{
			if(used_string_concatenation) maybe_bootstrap_error("string literal concatenation");

			/* Step past the leading '"' */
			j = 1;

			/* Copy the rest of the string as is */
			while(0 != global_token->s[j])
			{
				require(i < MAX_STRING, "concat string exceeded max string length\n");
				s[i] = global_token->s[j];
				i = i + 1;
				j = j + 1;
			}

			/* Move on to the next token */
			require_extra_token();
			used_string_concatenation = TRUE;
		}

		/* Now use it */
		strings_list = emit(parse_string(s), strings_list);
	}
}

void primary_expr_char(void)
{
	emit_load_immediate(REGISTER_ZERO, escape_lookup(global_token->s + 1), "primary expr char");
	require_extra_token();
}

void primary_expr_number(char* s)
{
	emit_load_immediate(REGISTER_ZERO, strtoint(s), "primary expr number");
}

struct token_list* load_address_of_variable_into_register(int reg, char* s)
{
	struct token_list* variable = static_variable_lookup(s);
	if(NULL != variable)
	{
		current_target = variable->type;
		emit_load_named_immediate(reg, "GLOBAL_", variable->s, "global load");
		return variable;
	}

	variable = sym_lookup(s, function->locals);
	if(NULL != variable)
	{
		current_target = variable->type;
		emit_load_relative_to_register(reg, REGISTER_LOCALS, variable->depth, "local variable load");

		return variable;
	}

	variable = sym_lookup(s, function->arguments);
	if(NULL != variable)
	{
		current_target = variable->type;
		emit_load_relative_to_register(reg, REGISTER_BASE, variable->depth, "function argument load");

		return variable;
	}

	variable = sym_lookup(s, global_function_list);
	if(NULL != variable)
	{
		emit_load_named_immediate(REGISTER_ZERO, "FUNCTION_", variable->s, "function load");
		return variable;
	}

	variable = sym_lookup(s, global_symbol_list);
	if(NULL != variable)
	{
		current_target = variable->type;
		emit_load_named_immediate(reg, "GLOBAL_", variable->s, "global load");
		return variable;
	}

	line_error();
	fputs(s ,stderr);
	fputs(" is not a defined symbol\n", stderr);
	exit(EXIT_FAILURE);
}

void emit_va_start_intrinsic(void)
{
	emit_out("# __va_start intrinsic\n");
	require_match("Invalid token after __va_start, expected '('", "(");

	require_token();
	char* ap_name = global_token->s;

	require_extra_token();

	require_match("Invalid token in __va_start, expected ','", ",");

	require_token();
	char* variable_name = global_token->s;
	require_extra_token();

	require_match("Invalid token at end of __va_start, expected ')'", ")");

	struct token_list* loaded = load_address_of_variable_into_register(REGISTER_ZERO, variable_name);
	if(stack_direction == STACK_DIRECTION_PLUS)
	{
		emit_add_immediate(REGISTER_ZERO, loaded->type->size, "Add size of variable");
	}
	else
	{
		emit_sub_immediate(REGISTER_ZERO, loaded->type->size, "Subtract size of variable");
	}

	load_address_of_variable_into_register(REGISTER_ONE, ap_name);

	/* Store REGISTER_ZERO in REGISTER_ONE deref */
	emit_out(store_value(register_size));
	emit_out("# __va_start intrinsic end\n");
}

void emit_va_arg_intrinsic(void)
{
	emit_out("# __va_arg intrinsic\n");
	require_match("Invalid token after __va_arg, expected '('", "(");

	require_token();
	char* ap_name = global_token->s;

	require_extra_token();

	require_match("Invalid token in __va_arg, expected ','", ",");

	require_token();
	struct type* type_size = type_name();

	require_match("Invalid token at end of __va_start, expected ')'", ")");

	emit_out("# REGISTER_ZERO = *ap\n");
	load_address_of_variable_into_register(REGISTER_ZERO, ap_name);
	emit_dereference(REGISTER_ZERO, "Deref ap");
	emit_dereference(REGISTER_ZERO, "Deref ap");
	emit_push(REGISTER_ZERO, "Dereffed va_arg");

	emit_out("# ap = ap - sizeof(ty)\n");
	load_address_of_variable_into_register(REGISTER_ZERO, ap_name);
	emit_push(REGISTER_ZERO, "Push ap address");
	emit_dereference(REGISTER_ZERO, "Deref ap for pointer to va_arg");
	if(stack_direction == STACK_DIRECTION_PLUS)
	{
		emit_add_immediate(REGISTER_ZERO, type_size->size, "Add size of variable");
	}
	else
	{
		emit_sub_immediate(REGISTER_ZERO, type_size->size, "Subtract size of variable");
	}

	emit_pop(REGISTER_ONE, "Pop AP address");
	/* Store REGISTER_ZERO in REGISTER_ONE deref */
	emit_out(store_value(type_size->size));

	emit_pop(REGISTER_ZERO, "Dereffed va_arg");

	emit_out("# __va_arg intrinsic end\n");
}

void emit_va_end_intrinsic(void)
{
	/* va_end is a noop for our impl */
	require_match("Invalid token after __va_arg, expected '('", "(");
	require_extra_token();
	require_match("Invalid token at end of __va_start, expected ')'", ")");
}

int num_dereference_after_postfix;
void primary_expr_variable(void)
{
	int num_dereference = 0;
	while(global_token->s[0] == '*') {
		require_extra_token();
		num_dereference = num_dereference + 1;
	}
	num_dereference_after_postfix = num_dereference;

	struct type* cast_type = NULL;
	if(global_token->s[0] == '(')
	{
		require_extra_token();

		cast_type = type_name();

		require_match("Expected token ')' in type cast.\n", ")");
	}

	char* s = global_token->s;
	require_extra_token();

	if(match("__va_start", s))
	{
		emit_va_start_intrinsic();
		return;
	}
	else if(match("__va_arg", s))
	{
		emit_va_arg_intrinsic();
		return;
	}
	else if(match("__va_end", s))
	{
		emit_va_end_intrinsic();
		return;
	}
	else if(match("__va_copy", s))
	{
		emit_va_end_intrinsic();
		return;
	}

	struct token_list* a = sym_lookup(s, global_constant_list);
	if(NULL != a)
	{
		emit_load_immediate(REGISTER_ZERO, strtoint(a->arguments->s), "constant load");
		return;
	}

	struct token_list* type = load_address_of_variable_into_register(REGISTER_ZERO, s);

	if(cast_type != NULL)
	{
		current_target = cast_type;
	}

	if(TRUE == Address_of) return;

	int options = type->options;

	if(match(".", global_token->s))
	{
		postfix_expr_stub();
		return;
	}

	int is_prefix_operator = (match("++", global_token->prev->prev->s) || match("--", global_token->prev->prev->s)) && (options != TLO_STATIC && options != TLO_GLOBAL);
	int is_postfix_operator = (match("++", global_token->s) || match("--", global_token->s)) && (options != TLO_STATIC && options != TLO_GLOBAL);
	int is_local_array = match("[", global_token->s) && (options & TLO_LOCAL_ARRAY);
	int is_function = options & TLO_FUNCTION;

	if(is_prefix_operator || is_postfix_operator || is_local_array || is_function)
	{
		return;
	}

	int is_assignment = match("=", global_token->s);
	int is_compound_operator = is_compound_assignment(global_token->s);

	if(!is_assignment && !is_compound_operator)
	{
		int size = register_size;
		if(options == TLO_LOCAL || options == TLO_ARGUMENT)
		{
			size = current_target->size;
		}
		emit_out(load_value(size, current_target->is_signed));
	}

	if(is_assignment)
	{
		while (num_dereference > 0)
		{
			emit_out(load_value(current_target->size, current_target->is_signed));
			current_target = current_target->type;
			num_dereference = num_dereference - 1;
		}
	}
	else
	{
		int should_not_deref;
		while (num_dereference > 0)
		{
			/* Function pointers are special in C.
			 * They can be dereferenced an infinite amount of times but still just be the actual pointer. */
			should_not_deref = current_target->type == current_target->type->type && (current_target->type->options & TO_FUNCTION_POINTER);

			if(!should_not_deref)
			{
				current_target = current_target->type;
				emit_out(load_value(current_target->size, current_target->is_signed));
			}
			num_dereference = num_dereference - 1;
		}
	}
}

void primary_expr(void);
struct type* promote_type(struct type* a, struct type* b)
{
	require(NULL != b, "impossible case 1 in promote_type\n");
	require(NULL != a, "impossible case 2 in promote_type\n");

	if(a == b) return a;

	struct type* i;
	for(i = global_types; NULL != i; i = i->next)
	{
		if(a->name == i->name) break;
		if(b->name == i->name) break;
		if(a->name == i->indirect->name) break;
		if(b->name == i->indirect->name) break;
		if(a->name == i->indirect->indirect->name) break;
		if(b->name == i->indirect->indirect->name) break;
	}
	require(NULL != i, "impossible case 3 in promote_type\n");
	return i;
}

void common_recursion(FUNCTION f)
{
	emit_push(REGISTER_ZERO, "_common_recursion");

	struct type* last_type = current_target;
	require_extra_token();
	f();
	current_target = promote_type(current_target, last_type);

	emit_pop(REGISTER_ONE, "_common_recursion");
}

void general_recursion(FUNCTION f, char* s, char* name, FUNCTION iterate)
{
	require(NULL != global_token, "Received EOF in general_recursion\n");
	if(match(name, global_token->s))
	{
		common_recursion(f);
		emit_out(s);
		iterate();
	}
}

void multiply_by_object_size(int object_size)
{
	/* bootstrap mode can't depend upon on pointer arithmetic */
	if(BOOTSTRAP_MODE) return;

	if(object_size == 1)
	{
		/* No reason to multiply by one */
		return;
	}

	emit_mul_register_zero_with_immediate(current_target->type->size, "pointer arithmetic");
}

void arithmetic_recursion(FUNCTION f, char* s1, char* s2, char* name, FUNCTION iterate)
{
	require(NULL != global_token, "Received EOF in arithmetic_recursion\n");
	if(match(name, global_token->s))
	{
		common_recursion(f);
		if(NULL == current_target)
		{
			emit_out(s1);
		}
		else if(current_target->is_signed)
		{
			emit_out(s1);
		}
		else
		{
			emit_out(s2);
		}
		iterate();
	}
}


/*
 * postfix-expr:
 *         primary-expr
 *         postfix-expr [ expression ]
 *         postfix-expr ( expression-list-opt )
 *         postfix-expr -> member
 *         postfix-expr . member
 */
struct type* lookup_member(struct type* parent, char* name);
void postfix_expr_arrow(void)
{
	emit_out("# looking up offset\n");
	require_extra_token();

	struct type* i = lookup_member(current_target, global_token->s);
	current_target = i->type;
	require_extra_token();

	if(0 != i->offset)
	{
		emit_add_immediate(REGISTER_ZERO, i->offset, "-> offset calculation");
	}

	if(global_token->s[0] == '.') return;

	int is_postfix_operator = match("++", global_token->s) || match("--", global_token->s);
	/* We don't yet support assigning structs to structs */
	if((!match("=", global_token->s) && !is_compound_assignment(global_token->s) && !is_postfix_operator && (register_size >= i->size)))
	{
		emit_out(load_value(i->size, i->is_signed));
	}
}

void postfix_expr_inc_or_dec(void)
{
	int is_subtract = global_token->s[0] == '-';
	require_extra_token();

	emit_out("# postfix inc/dec\n");
	emit_push(REGISTER_ONE, "Old register one value");

	emit_push(REGISTER_ZERO, "Address of variable");
	emit_dereference(REGISTER_ZERO, "Get value");

	/* We need the address to be at the top of the stack and the value to be below it */
	emit_pop(REGISTER_ONE, "Address of variable");
	emit_push(REGISTER_ZERO, "Value before postfix operator");
	emit_push(REGISTER_ONE, "Address of variable");

	int value = 1;
	if(type_is_pointer(current_target))
	{
		value = current_target->type->size;
	}

	if(is_subtract)
	{
		emit_sub_immediate(REGISTER_ZERO, value, "Subtract offset");
	}
	else
	{
		emit_add_immediate(REGISTER_ZERO, value, "Add offset");
	}

	emit_pop(REGISTER_ONE, "Address of variable");

	/* Store REGISTER_ZERO in REGISTER_ONE deref */
	emit_out(store_value(current_target->size));

	emit_pop(REGISTER_ZERO, "Value before postfix operator");
	emit_pop(REGISTER_ONE, "Previous value");

	while (num_dereference_after_postfix > 0)
	{
		emit_out(load_value(current_target->type->size, current_target->type->is_signed));
		current_target = current_target->type;
		num_dereference_after_postfix = num_dereference_after_postfix - 1;
	}

	emit_out("# postfix inc/dec end\n");
}

void postfix_expr_dot(void)
{
	maybe_bootstrap_error("Member access using .");
	emit_out("# looking up offset\n");
	require_extra_token();

	struct type* i = lookup_member(current_target, global_token->s);
	current_target = i->type;
	require_extra_token();

	if(0 != i->offset)
	{
		emit_add_immediate(REGISTER_ZERO, i->offset, ". offset calculation");
	}
	int is_postfix_operator = match("++", global_token->s) || match("--", global_token->s);
	if(match("=", global_token->s) || is_compound_assignment(global_token->s) || is_postfix_operator) return;
	if(match("[", global_token->s) || match(".", global_token->s)) return;

	emit_out(load_value(current_target->size, current_target->is_signed));
}

void postfix_expr_array(void)
{
	char* prefix_operator = global_token->prev->prev->s;

	struct type* array = current_target;
	common_recursion(expression);
	current_target = array;
	require(NULL != current_target, "Arrays only apply to variables\n");

	char* assign = load_value(register_size, current_target->is_signed);

	/* Add support for Ints */
	if(match("char*", current_target->name))
	{
		assign = load_value(1, TRUE);
	}
	else
	{
		emit_mul_register_zero_with_immediate(current_target->type->size, "primary expr array");
	}

	emit_add(REGISTER_ZERO, REGISTER_ONE, TRUE, "primary expr array");

	require_match("ERROR in postfix_expr\nMissing ]\n", "]");
	require(NULL != global_token, "truncated array expression\n");

	int is_prefix_operator = match("++", prefix_operator) || match("--", prefix_operator);
	int is_postfix_operator = match("++", global_token->s) || match("--", global_token->s);
	if(match("=", global_token->s) || is_compound_assignment(global_token->s) || match(".", global_token->s) || is_prefix_operator || is_postfix_operator)
	{
		assign = "";
	}
	if(match("[", global_token->s))
	{
		current_target = current_target->type;
	}

	emit_out(assign);
}

/*
 * unary-expr:
 *         &postfix-expr
 *         - postfix-expr
 *         !postfix-expr
 *         sizeof ( type )
 */
int unary_expr_sizeof(void)
{
	require_extra_token();
	require_match("ERROR in unary_expr\nMissing (\n", "(");
	struct token_list* t = NULL;

	int num_dereferences = 0;
	if(!BOOTSTRAP_MODE)
	{
		if(global_token->s[0] == '\"')
		{
			/* string_length (strlen) doesn't include the null terminator but the
			 * token starts with a quotation mark so the count will be correct. */
			int length = string_length(global_token->s);

			require_extra_token();

			require_match("ERROR in unary_expr\nMissing )\n", ")");

			return length;
		}

		while(global_token->s[0] == '*')
		{
			num_dereferences = num_dereferences + 1;
			require_extra_token();
		}

		t = static_variable_lookup(global_token->s);
		if(NULL == t)
		{
			t = sym_lookup(global_token->s, global_constant_list);
			if(NULL == t && NULL != function)
			{
				t = sym_lookup(global_token->s, function->locals);
				if(NULL == t)
				{
					t = sym_lookup(global_token->s, function->arguments);
				}
			}
		}

		if(NULL == t)
		{
			t = sym_lookup(global_token->s, global_symbol_list);
		}
	}

	int size = 0;
	if(t != NULL)
	{
		require_extra_token();

		struct type* a = t->type;
		while(num_dereferences > 0)
		{
			a = a->type;
			num_dereferences = num_dereferences - 1;
		}

		if(t->array_modifier != 0)
		{
			size = a->size * t->array_modifier;
		}
		else size = a->size;
	}
	else
	{
		if(num_dereferences != 0)
		{
			line_error();
			fputs("Unable to dereference type for sizeof.\n", stderr);
			exit(EXIT_FAILURE);
		}

		struct type* a = type_name();
		size = a->size;
	}

	require_match("ERROR in unary_expr\nMissing )\n", ")");

	return size;
}

void postfix_expr_stub(void)
{
	require(NULL != global_token, "Unexpected EOF, improperly terminated primary expression\n");
	if(match("[", global_token->s))
	{
		postfix_expr_array();
		postfix_expr_stub();
	}

	if(match("->", global_token->s))
	{
		postfix_expr_arrow();
		postfix_expr_stub();
	}

	if(match(".", global_token->s))
	{
		postfix_expr_dot();
		postfix_expr_stub();
	}

	if(match("++", global_token->s) || match("--", global_token->s))
	{
		postfix_expr_inc_or_dec();
		postfix_expr_stub();
	}

	if(global_token->s[0] == '(')
	{
		if((current_target->options & TO_FUNCTION_POINTER))
		{
			function_call(NULL, TRUE);
		}
		else
		{
			line_error();
			fputs("Attempted to use operator ( on non-function pointer", stderr);
			exit(EXIT_FAILURE);
		}
	}
}

void postfix_expr(void)
{
	primary_expr();
	postfix_expr_stub();
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
void additive_expr_stub_a(void)
{
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		arithmetic_recursion(postfix_expr, "MUL R0 R1 R0\n", "MULU R0 R1 R0\n", "*", additive_expr_stub_a);
		arithmetic_recursion(postfix_expr, "DIV R0 R1 R0\n", "DIVU R0 R1 R0\n", "/", additive_expr_stub_a);
		arithmetic_recursion(postfix_expr, "MOD R0 R1 R0\n", "MODU R0 R1 R0\n", "%", additive_expr_stub_a);
	}
	else if(X86 == Architecture)
	{
		arithmetic_recursion(postfix_expr, "imul_ebx\n", "mul_ebx\n", "*", additive_expr_stub_a);
		arithmetic_recursion(postfix_expr, "xchg_ebx,eax\ncdq\nidiv_ebx\n", "xchg_ebx,eax\nxor_edx,edx\ndiv_ebx\n", "/", additive_expr_stub_a);
		arithmetic_recursion(postfix_expr, "xchg_ebx,eax\ncdq\nidiv_ebx\nmov_eax,edx\n", "xchg_ebx,eax\nxor_edx,edx\ndiv_ebx\nmov_eax,edx\n", "%", additive_expr_stub_a);
	}
	else if(AMD64 == Architecture)
	{
		arithmetic_recursion(postfix_expr, "imul_rbx\n", "mul_rbx\n", "*", additive_expr_stub_a);
		arithmetic_recursion(postfix_expr, "xchg_rbx,rax\ncqo\nidiv_rbx\n", "xchg_rbx,rax\nxor_edx,edx\ndiv_rbx\n", "/", additive_expr_stub_a);
		arithmetic_recursion(postfix_expr, "xchg_rbx,rax\ncqo\nidiv_rbx\nmov_rax,rdx\n", "xchg_rbx,rax\nxor_edx,edx\ndiv_rbx\nmov_rax,rdx\n", "%", additive_expr_stub_a);
	}
	else if(ARMV7L == Architecture)
	{
		arithmetic_recursion(postfix_expr, "'9' R0 '0' R1 MULS R0 ARITH2_ALWAYS\n", "'9' R0 '0' R1 MUL R0 ARITH2_ALWAYS\n", "*", additive_expr_stub_a);
		arithmetic_recursion(postfix_expr, "{LR} PUSH_ALWAYS\n^~divides CALL_ALWAYS\n{LR} POP_ALWAYS\n", "{LR} PUSH_ALWAYS\n^~divide CALL_ALWAYS\n{LR} POP_ALWAYS\n", "/", additive_expr_stub_a);
		arithmetic_recursion(postfix_expr, "{LR} PUSH_ALWAYS\n^~moduluss CALL_ALWAYS\n{LR} POP_ALWAYS\n", "{LR} PUSH_ALWAYS\n^~modulus CALL_ALWAYS\n{LR} POP_ALWAYS\n", "%", additive_expr_stub_a);
	}
	else if(AARCH64 == Architecture)
	{
		general_recursion(postfix_expr, "MUL_X0_X1_X0\n", "*", additive_expr_stub_a);
		arithmetic_recursion(postfix_expr, "SDIV_X0_X1_X0\n", "UDIV_X0_X1_X0\n", "/", additive_expr_stub_a);
		arithmetic_recursion(postfix_expr, "SDIV_X2_X1_X0\nMSUB_X0_X0_X2_X1\n", "UDIV_X2_X1_X0\nMSUB_X0_X0_X2_X1\n", "%", additive_expr_stub_a);
	}
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
	{
		general_recursion(postfix_expr, "rd_a0 rs1_a1 rs2_a0 mul\n", "*", additive_expr_stub_a);
		arithmetic_recursion(postfix_expr, "rd_a0 rs1_a1 rs2_a0 div\n", "rd_a0 rs1_a1 rs2_a0 divu\n", "/", additive_expr_stub_a);
		arithmetic_recursion(postfix_expr, "rd_a0 rs1_a1 rs2_a0 rem\n", "rd_a0 rs1_a1 rs2_a0 remu\n", "%", additive_expr_stub_a);
	}
}


void additive_expr_a(void)
{
	postfix_expr();
	additive_expr_stub_a();
}

void additive_expr_stub_b(void)
{
	require(NULL != global_token, "Received EOF in additive_expr_stub_a\n");
	if(match("+", global_token->s))
	{
		common_recursion(additive_expr_a);
		emit_add(REGISTER_ZERO, REGISTER_ONE, current_target->is_signed, NULL);
		additive_expr_stub_b();
	}
	else if(match("-", global_token->s))
	{
		common_recursion(additive_expr_a);
		emit_rsub(REGISTER_ZERO, REGISTER_ONE, current_target->is_signed, NULL);
		additive_expr_stub_b();
	}
}


void additive_expr_b(void)
{
	additive_expr_a();
	additive_expr_stub_b();
}

void additive_expr_stub_c(void)
{
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		arithmetic_recursion(additive_expr_b, "SAL R0 R1 R0\n", "SL0 R0 R1 R0\n", "<<", additive_expr_stub_c);
		arithmetic_recursion(additive_expr_b, "SAR R0 R1 R0\n", "SR0 R0 R1 R0\n", ">>", additive_expr_stub_c);
	}
	else if(X86 == Architecture)
	{
		arithmetic_recursion(additive_expr_b, "mov_ecx,eax\nmov_eax,ebx\nsal_eax,cl\n", "mov_ecx,eax\nmov_eax,ebx\nshl_eax,cl\n", "<<", additive_expr_stub_c);
		arithmetic_recursion(additive_expr_b, "mov_ecx,eax\nmov_eax,ebx\nsar_eax,cl\n", "mov_ecx,eax\nmov_eax,ebx\nshr_eax,cl\n", ">>", additive_expr_stub_c);
	}
	else if(AMD64 == Architecture)
	{
		arithmetic_recursion(additive_expr_b, "mov_rcx,rax\nmov_rax,rbx\nsal_rax,cl\n", "mov_rcx,rax\nmov_rax,rbx\nshl_rax,cl\n", "<<", additive_expr_stub_c);
		arithmetic_recursion(additive_expr_b, "mov_rcx,rax\nmov_rax,rbx\nsar_rax,cl\n", "mov_rcx,rax\nmov_rax,rbx\nshr_rax,cl\n", ">>", additive_expr_stub_c);
	}
	else if(ARMV7L == Architecture)
	{
		arithmetic_recursion(additive_expr_b, "LEFT R1 R0 R0 SHIFT AUX_ALWAYS\n", "LEFT R1 R0 R0 SHIFT AUX_ALWAYS\n", "<<", additive_expr_stub_c);
		arithmetic_recursion(additive_expr_b, "ARITH_RIGHT R1 R0 R0 SHIFT AUX_ALWAYS\n", "RIGHT R1 R0 R0 SHIFT AUX_ALWAYS\n", ">>", additive_expr_stub_c);
	}
	else if(AARCH64 == Architecture)
	{
		general_recursion(additive_expr_b, "LSHIFT_X0_X1_X0\n", "<<", additive_expr_stub_c);
		arithmetic_recursion(additive_expr_b, "ARITH_RSHIFT_X0_X1_X0\n", "LOGICAL_RSHIFT_X0_X1_X0\n", ">>", additive_expr_stub_c);
	}
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
	{
		general_recursion(additive_expr_b, "rd_a0 rs1_a1 rs2_a0 sll\n", "<<", additive_expr_stub_c);
		arithmetic_recursion(additive_expr_b, "rd_a0 rs1_a1 rs2_a0 sra\n", "rd_a0 rs1_a1 rs2_a0 srl\n", ">>", additive_expr_stub_c);
	}
}


void additive_expr_c(void)
{
	additive_expr_b();
	additive_expr_stub_c();
}


/*
 * relational-expr:
 *         additive_expr
 *         relational-expr < additive_expr
 *         relational-expr <= additive_expr
 *         relational-expr >= additive_expr
 *         relational-expr > additive_expr
 */

void relational_expr_stub(void)
{
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		arithmetic_recursion(additive_expr_c, "CMP R0 R1 R0\nSET.L R0 R0 1\n", "CMPU R0 R1 R0\nSET.L R0 R0 1\n", "<", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "CMP R0 R1 R0\nSET.LE R0 R0 1\n", "CMPU R0 R1 R0\nSET.LE R0 R0 1\n", "<=", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "CMP R0 R1 R0\nSET.GE R0 R0 1\n", "CMPU R0 R1 R0\nSET.GE R0 R0 1\n", ">=", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "CMP R0 R1 R0\nSET.G R0 R0 1\n", "CMPU R0 R1 R0\nSET.G R0 R0 1\n", ">", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "CMP R0 R1 R0\nSET.E R0 R0 1\n", "CMPU R0 R1 R0\nSET.E R0 R0 1\n", "==", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "CMP R0 R1 R0\nSET.NE R0 R0 1\n", "CMPU R0 R1 R0\nSET.NE R0 R0 1\n", "!=", relational_expr_stub);
	}
	else if(X86 == Architecture)
	{
		arithmetic_recursion(additive_expr_c, "cmp_ebx,eax\nsetl_al\nmovzx_eax,al\n", "cmp_ebx,eax\nsetb_al\nmovzx_eax,al\n", "<", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "cmp_ebx,eax\nsetle_al\nmovzx_eax,al\n", "cmp_ebx,eax\nsetbe_al\nmovzx_eax,al\n", "<=", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "cmp_ebx,eax\nsetge_al\nmovzx_eax,al\n", "cmp_ebx,eax\nsetae_al\nmovzx_eax,al\n", ">=", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "cmp_ebx,eax\nsetg_al\nmovzx_eax,al\n", "cmp_ebx,eax\nseta_al\nmovzx_eax,al\n", ">", relational_expr_stub);
		general_recursion(additive_expr_c, "cmp_ebx,eax\nsete_al\nmovzx_eax,al\n", "==", relational_expr_stub);
		general_recursion(additive_expr_c, "cmp_ebx,eax\nsetne_al\nmovzx_eax,al\n", "!=", relational_expr_stub);
	}
	else if(AMD64 == Architecture)
	{
		arithmetic_recursion(additive_expr_c, "cmp_rbx,rax\nsetl_al\nmovzx_rax,al\n", "cmp_rbx,rax\nsetb_al\nmovzx_rax,al\n", "<", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "cmp_rbx,rax\nsetle_al\nmovzx_rax,al\n", "cmp_rbx,rax\nsetbe_al\nmovzx_rax,al\n", "<=", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "cmp_rbx,rax\nsetge_al\nmovzx_rax,al\n", "cmp_rbx,rax\nsetae_al\nmovzx_rax,al\n", ">=", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "cmp_rbx,rax\nsetg_al\nmovzx_rax,al\n", "cmp_rbx,rax\nseta_al\nmovzx_rax,al\n", ">", relational_expr_stub);
		general_recursion(additive_expr_c, "cmp_rbx,rax\nsete_al\nmovzx_rax,al\n", "==", relational_expr_stub);
		general_recursion(additive_expr_c, "cmp_rbx,rax\nsetne_al\nmovzx_rax,al\n", "!=", relational_expr_stub);
	}
	else if(ARMV7L == Architecture)
	{
		arithmetic_recursion(additive_expr_c, "'0' R0 CMP R1 AUX_ALWAYS\n!0 R0 LOADI8_ALWAYS\n!1 R0 LOADI8_L\n", "'0' R0 CMP R1 AUX_ALWAYS\n!0 R0 LOADI8_ALWAYS\n!1 R0 LOADI8_LO\n", "<", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "'0' R0 CMP R1 AUX_ALWAYS\n!0 R0 LOADI8_ALWAYS\n!1 R0 LOADI8_LE\n", "'0' R0 CMP R1 AUX_ALWAYS\n!0 R0 LOADI8_ALWAYS\n!1 R0 LOADI8_LS\n", "<=", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "'0' R0 CMP R1 AUX_ALWAYS\n!0 R0 LOADI8_ALWAYS\n!1 R0 LOADI8_GE\n", "'0' R0 CMP R1 AUX_ALWAYS\n!0 R0 LOADI8_ALWAYS\n!1 R0 LOADI8_HS\n", ">=", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "'0' R0 CMP R1 AUX_ALWAYS\n!0 R0 LOADI8_ALWAYS\n!1 R0 LOADI8_G\n", "'0' R0 CMP R1 AUX_ALWAYS\n!0 R0 LOADI8_ALWAYS\n!1 R0 LOADI8_HI\n", ">", relational_expr_stub);
		general_recursion(additive_expr_c, "'0' R0 CMP R1 AUX_ALWAYS\n!0 R0 LOADI8_ALWAYS\n!1 R0 LOADI8_EQUAL\n", "==", relational_expr_stub);
		general_recursion(additive_expr_c, "'0' R0 CMP R1 AUX_ALWAYS\n!0 R0 LOADI8_ALWAYS\n!1 R0 LOADI8_NE\n", "!=", relational_expr_stub);
	}
	else if(AARCH64 == Architecture)
	{
		arithmetic_recursion(additive_expr_c, "CMP_X1_X0\nSET_X0_TO_1\nSKIP_INST_LT\nSET_X0_TO_0\n", "CMP_X1_X0\nSET_X0_TO_1\nSKIP_INST_LO\nSET_X0_TO_0\n", "<", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "CMP_X1_X0\nSET_X0_TO_1\nSKIP_INST_LE\nSET_X0_TO_0\n", "CMP_X1_X0\nSET_X0_TO_1\nSKIP_INST_LS\nSET_X0_TO_0\n", "<=", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "CMP_X1_X0\nSET_X0_TO_1\nSKIP_INST_GE\nSET_X0_TO_0\n", "CMP_X1_X0\nSET_X0_TO_1\nSKIP_INST_HS\nSET_X0_TO_0\n", ">=", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "CMP_X1_X0\nSET_X0_TO_1\nSKIP_INST_GT\nSET_X0_TO_0\n", "CMP_X1_X0\nSET_X0_TO_1\nSKIP_INST_HI\nSET_X0_TO_0\n", ">", relational_expr_stub);
		general_recursion(additive_expr_c, "CMP_X1_X0\nSET_X0_TO_1\nSKIP_INST_EQ\nSET_X0_TO_0\n", "==", relational_expr_stub);
		general_recursion(additive_expr_c, "CMP_X1_X0\nSET_X0_TO_1\nSKIP_INST_NE\nSET_X0_TO_0\n", "!=", relational_expr_stub);
	}
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
	{
		arithmetic_recursion(additive_expr_c, "rd_a0 rs1_a1 rs2_a0 slt\n", "rd_a0 rs1_a1 rs2_a0 sltu\n", "<", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "rd_a0 rs1_a0 rs2_a1 slt\nrd_a0 rs1_a0 !1 xori\n", "rd_a0 rs1_a0 rs2_a1 sltu\nrd_a0 rs1_a0 !1 xori\n", "<=", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "rd_a0 rs1_a1 rs2_a0 slt\nrd_a0 rs1_a0 !1 xori\n", "rd_a0 rs1_a1 rs2_a0 sltu\nrd_a0 rs1_a0 !1 xori\n", ">=", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "rd_a0 rs1_a0 rs2_a1 slt\n", "rd_a0 rs1_a0 rs2_a1 sltu\n", ">", relational_expr_stub);
		general_recursion(additive_expr_c, "rd_a0 rs1_a0 rs2_a1 sub\nrd_a0 rs1_a0 !1 sltiu\n", "==", relational_expr_stub);
		general_recursion(additive_expr_c, "rd_a0 rs1_a0 rs2_a1 sub\nrd_a0 rs2_a0 sltu\n", "!=", relational_expr_stub);
	}
}

void relational_expr(void)
{
	additive_expr_c();
	relational_expr_stub();
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
void bitwise_expr_stub(void)
{
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		general_recursion(relational_expr, "AND R0 R0 R1\n", "&", bitwise_expr_stub);
		general_recursion(relational_expr, "AND R0 R0 R1\n", "&&", bitwise_expr_stub);
		general_recursion(relational_expr, "OR R0 R0 R1\n", "|", bitwise_expr_stub);
		general_recursion(relational_expr, "OR R0 R0 R1\n", "||", bitwise_expr_stub);
		general_recursion(relational_expr, "XOR R0 R0 R1\n", "^", bitwise_expr_stub);
	}
	else if(X86 == Architecture)
	{
		general_recursion(relational_expr, "and_eax,ebx\n", "&", bitwise_expr_stub);
		general_recursion(relational_expr, "and_eax,ebx\n", "&&", bitwise_expr_stub);
		general_recursion(relational_expr, "or_eax,ebx\n", "|", bitwise_expr_stub);
		general_recursion(relational_expr, "or_eax,ebx\n", "||", bitwise_expr_stub);
		general_recursion(relational_expr, "xor_eax,ebx\n", "^", bitwise_expr_stub);
	}
	else if(AMD64 == Architecture)
	{
		general_recursion(relational_expr, "and_rax,rbx\n", "&", bitwise_expr_stub);
		general_recursion(relational_expr, "and_rax,rbx\n", "&&", bitwise_expr_stub);
		general_recursion(relational_expr, "or_rax,rbx\n", "|", bitwise_expr_stub);
		general_recursion(relational_expr, "or_rax,rbx\n", "||", bitwise_expr_stub);
		general_recursion(relational_expr, "xor_rax,rbx\n", "^", bitwise_expr_stub);
	}
	else if(ARMV7L == Architecture)
	{
		general_recursion(relational_expr, "NO_SHIFT R0 R0 AND R1 ARITH2_ALWAYS\n", "&", bitwise_expr_stub);
		general_recursion(relational_expr, "NO_SHIFT R0 R0 AND R1 ARITH2_ALWAYS\n", "&&", bitwise_expr_stub);
		general_recursion(relational_expr, "NO_SHIFT R0 R0 OR R1 AUX_ALWAYS\n", "|", bitwise_expr_stub);
		general_recursion(relational_expr, "NO_SHIFT R0 R0 OR R1 AUX_ALWAYS\n", "||", bitwise_expr_stub);
		general_recursion(relational_expr, "'0' R0 R0 XOR R1 ARITH2_ALWAYS\n", "^", bitwise_expr_stub);
	}
	else if(AARCH64 == Architecture)
	{
		general_recursion(relational_expr, "AND_X0_X1_X0\n", "&", bitwise_expr_stub);
		general_recursion(relational_expr, "AND_X0_X1_X0\n", "&&", bitwise_expr_stub);
		general_recursion(relational_expr, "OR_X0_X1_X0\n", "|", bitwise_expr_stub);
		general_recursion(relational_expr, "OR_X0_X1_X0\n", "||", bitwise_expr_stub);
		general_recursion(relational_expr, "XOR_X0_X1_X0\n", "^", bitwise_expr_stub);
	}
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
	{
		general_recursion(relational_expr, "rd_a0 rs1_a1 rs2_a0 and\n", "&", bitwise_expr_stub);
		general_recursion(relational_expr, "rd_a0 rs1_a1 rs2_a0 and\n", "&&", bitwise_expr_stub);
		general_recursion(relational_expr, "rd_a0 rs1_a1 rs2_a0 or\n", "|", bitwise_expr_stub);
		general_recursion(relational_expr, "rd_a0 rs1_a1 rs2_a0 or\n", "||", bitwise_expr_stub);
		general_recursion(relational_expr, "rd_a0 rs1_a1 rs2_a0 xor\n", "^", bitwise_expr_stub);
	}
}


void bitwise_expr(void)
{
	relational_expr();
	bitwise_expr_stub();
}

/*
 * expression:
 *         bitwise-or-expr
 *         bitwise-or-expr = expression
 */

void primary_expr(void)
{
	require(NULL != global_token, "Received EOF where primary expression expected\n");
	if(match("&", global_token->s))
	{
		Address_of = TRUE;
		require_extra_token();
	}
	else
	{
		Address_of = FALSE;
	}

	if(match("sizeof", global_token->s)) emit_load_immediate(REGISTER_ZERO, unary_expr_sizeof(), "load sizeof");
	else if(match("-", global_token->s))
	{
		if((KNIGHT_POSIX != Architecture) && (KNIGHT_NATIVE != Architecture))
		{
			emit_load_immediate(REGISTER_ZERO, 0, "primary expr");
		}

		common_recursion(primary_expr);

		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("NEG R0 R0\n");
		else if(X86 == Architecture) emit_out("sub_ebx,eax\nmov_eax,ebx\n");
		else if(AMD64 == Architecture) emit_out("sub_rbx,rax\nmov_rax,rbx\n");
		else if(ARMV7L == Architecture) emit_out("'0' R0 R0 SUB R1 ARITH2_ALWAYS\n");
		else if(AARCH64 == Architecture) emit_out("SUB_X0_X1_X0\n");
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("rd_a0 rs1_a1 rs2_a0 sub\n");
	}
	else if('!' == global_token->s[0])
	{
		if((RISCV32 != Architecture) && (RISCV64 != Architecture))
		{
			emit_load_immediate(REGISTER_ZERO, 1, "primary expr");
		}

		common_recursion(postfix_expr);

		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("CMPU R0 R1 R0\nSET.G R0 R0 1\n");
		else if(X86 == Architecture) emit_out("cmp_ebx,eax\nseta_al\nmovzx_eax,al\n");
		else if(AMD64 == Architecture) emit_out("cmp_rbx,rax\nseta_al\nmovzx_rax,al\n");
		else if(ARMV7L == Architecture) emit_out("'0' R0 CMP R1 AUX_ALWAYS\n!0 R0 LOADI8_ALWAYS\n!1 R0 LOADI8_HI\n");
		else if(AARCH64 == Architecture) emit_out("CMP_X1_X0\nSET_X0_TO_1\nSKIP_INST_HI\nSET_X0_TO_0\n");
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("rd_a0 rs1_a0 !1 sltiu\n");
	}
	else if('~' == global_token->s[0])
	{
		common_recursion(postfix_expr);

		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("NOT R0 R0\n");
		else if(X86 == Architecture) emit_out("not_eax\n");
		else if(AMD64 == Architecture) emit_out("not_rax\n");
		else if(ARMV7L == Architecture) emit_out("'0' R0 R0 MVN_ALWAYS\n");
		else if(AARCH64 == Architecture) emit_out("MVN_X0\n");
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("rd_a0 rs1_a0 not\n");
	}
	else if(match("--", global_token->s) || match("++", global_token->s))
	{
		int is_subtract = global_token->s[0] == '-';
		maybe_bootstrap_error("prefix operators --/++");

		emit_out("# prefix inc/dec\n");

		emit_push(REGISTER_ZERO, "Previous value");
		require_extra_token();
		postfix_expr();
		emit_pop(REGISTER_ONE, "Restore previous value");

		emit_push(REGISTER_ONE, "Previous value");
		emit_push(REGISTER_ZERO, "Address of variable");

		emit_dereference(REGISTER_ZERO, "Deref to get value");

		int value = 1;
		if(type_is_pointer(current_target))
		{
			value = current_target->type->size;
		}

		if(is_subtract)
		{
			emit_sub_immediate(REGISTER_ZERO, value, "Sub prefix from deref value");
		}
		else
		{
			emit_add_immediate(REGISTER_ZERO, value, "Add prefix to deref value");
		}

		emit_pop(REGISTER_ONE, "Address of variable");

		/* Store REGISTER_ZERO in REGISTER_ONE deref */
		emit_out(store_value(current_target->size));

		emit_pop(REGISTER_ONE, "Previous value");

		emit_out("# prefix inc/dec end\n");
	}
	else if(global_token->s[0] == '(')
	{
		require_extra_token();
		struct type* type_size = fallible_type_name();
		if(type_size != NULL)
		{
			if(global_token->s[0] == '(')
			{
				parse_function_pointer();
				type_size = function_pointer;
			}
			require_match("Invalid character received in cast. Expected ')'.\n", ")");
			primary_expr();
			current_target = type_size;
		}
		else
		{
			expression();
			require_match("Error in Primary expression\nDidn't get )\n", ")");
		}
	}
	else if(global_token->s[0] == '\'') primary_expr_char();
	else if(global_token->s[0] == '"') primary_expr_string();
	else if(in_set(global_token->s[0], "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"))
	{
		struct token_list* variable = sym_lookup(global_token->s, global_function_list);
		if (variable != NULL && global_token->next->s[0] == '(')
		{
			/* Call function directly without loading into register optimization */
			require_extra_token();
			function_call(variable, FALSE);
		}
		else
		{
			primary_expr_variable();
		}
	}
	else if(global_token->s[0] == '*') primary_expr_variable();
	else if(in_set(global_token->s[0], "0123456789"))
	{
		primary_expr_number(global_token->s);
		require_extra_token();
	}
	else primary_expr_failure();
}

char* compound_operation(char* operator, int is_signed)
{
	char* operation = "";
	if(match("+=", operator))
	{
		emit_add(REGISTER_ZERO, REGISTER_ONE, is_signed, "compound operation");
	}
	else if(match("-=", operator))
	{
		emit_rsub(REGISTER_ZERO, REGISTER_ONE, is_signed, "compound operation");
	}
	else if(match("*=", operator))
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
		{
			if(is_signed) operation = "MUL R0 R1 R0\n";
			else operation =  "MULU R0 R1 R0\n";
		}
		else if(X86 == Architecture)
		{
			if(is_signed) operation = "imul_ebx\n";
			else operation = "mul_ebx\n";
		}
		else if(AMD64 == Architecture)
		{
			if(is_signed) operation = "imul_rbx\n";
			else operation = "mul_rbx\n";
		}
		else if(ARMV7L == Architecture) operation = "'9' R0 '0' R1 MULS R0 ARITH2_ALWAYS\n";
		else if(AARCH64 == Architecture) operation = "MUL_X0_X1_X0\n";
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) operation = "rd_a0 rs1_a1 rs2_a0 mul\n";
	}
	else if(match("/=", operator))
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
		{
			if(is_signed) operation = "DIV R0 R1 R0\n";
			else operation =  "DIVU R0 R1 R0\n";
		}
		else if(X86 == Architecture)
		{
			if (is_signed) operation = "xchg_ebx,eax\ncdq\nidiv_ebx\n";
			else operation = "xchg_ebx,eax\nxor_edx,edx\ndiv_ebx\n";
		}
		else if(AMD64 == Architecture)
		{
			if(is_signed) operation = "xchg_rbx,rax\ncqo\nidiv_rbx\n";
			else operation = "xchg_rbx,rax\nxor_edx,edx\ndiv_rbx\n";
		}
		else if(ARMV7L == Architecture)
		{
			if(is_signed) operation = "{LR} PUSH_ALWAYS\n^~divides CALL_ALWAYS\n{LR} POP_ALWAYS\n";
			else operation = "{LR} PUSH_ALWAYS\n^~divide CALL_ALWAYS\n{LR} POP_ALWAYS\n";
		}
		else if(AARCH64 == Architecture)
		{
			if(is_signed) operation = "SDIV_X0_X1_X0\n";
			else operation = "UDIV_X0_X1_X0\n";
		}
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
		{
			if(is_signed) operation = "rd_a0 rs1_a1 rs2_a0 div\n";
			else operation = "rd_a0 rs1_a1 rs2_a0 divu\n";
		}
	}
	else if(match("%=", operator))
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
		{
			if(is_signed) operation = "MOD R0 R1 R0\n";
			else operation = "MODU R0 R1 R0\n";
		}
		else if(X86 == Architecture)
		{
			if(is_signed) operation = "xchg_ebx,eax\ncdq\nidiv_ebx\nmov_eax,edx\n";
			else operation = "xchg_ebx,eax\nxor_edx,edx\ndiv_ebx\nmov_eax,edx\n";
		}
		else if(AMD64 == Architecture)
		{
			if(is_signed) operation = "xchg_rbx,rax\ncqo\nidiv_rbx\nmov_rax,rdx\n";
			else operation = "xchg_rbx,rax\nxor_edx,edx\ndiv_rbx\nmov_rax,rdx\n";
		}
		else if(ARMV7L == Architecture)
		{
			if(is_signed) operation = "{LR} PUSH_ALWAYS\n^~moduluss CALL_ALWAYS\n{LR} POP_ALWAYS\n";
			else operation = "{LR} PUSH_ALWAYS\n^~modulus CALL_ALWAYS\n{LR} POP_ALWAYS\n";
		}
		else if(AARCH64 == Architecture)
		{
			if(is_signed) operation = "SDIV_X2_X1_X0\nMSUB_X0_X0_X2_X1\n";
			else operation = "UDIV_X2_X1_X0\nMSUB_X0_X0_X2_X1\n";
		}
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
		{
			if(is_signed) operation = "rd_a0 rs1_a1 rs2_a0 rem\n";
			else operation = "rd_a0 rs1_a1 rs2_a0 remu\n";
		}
	}
	else if(match("<<=", operator))
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
		{
			if(is_signed) operation = "SAL R0 R1 R0\n";
			else operation = "SL0 R0 R1 R0\n";
		}
		else if(X86 == Architecture)
		{
			if(is_signed) operation = "mov_ecx,eax\nmov_eax,ebx\nsal_eax,cl\n";
			else operation = "mov_ecx,eax\nmov_eax,ebx\nshl_eax,cl\n";
		}
		else if(AMD64 == Architecture)
		{
			if(is_signed) operation = "mov_rcx,rax\nmov_rax,rbx\nsal_rax,cl\n";
			else operation = "mov_rcx,rax\nmov_rax,rbx\nshl_rax,cl\n";
		}
		else if(ARMV7L == Architecture) operation = "LEFT R1 R0 R0 SHIFT AUX_ALWAYS\n";
		else if(AARCH64 == Architecture) operation = "LSHIFT_X0_X1_X0\n";
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) operation = "rd_a0 rs1_a1 rs2_a0 sll\n";
	}
	else if(match(">>=", operator))
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
		{
			if(is_signed) operation = "SAR R0 R1 R0\n";
			else operation = "SR0 R0 R1 R0\n";
		}
		else if(X86 == Architecture)
		{
			if(is_signed) operation = "mov_ecx,eax\nmov_eax,ebx\nsar_eax,cl\n";
			else operation = "mov_ecx,eax\nmov_eax,ebx\nshr_eax,cl\n";
		}
		else if(AMD64 == Architecture)
		{
			if(is_signed) operation = "mov_rcx,rax\nmov_rax,rbx\nsar_rax,cl\n";
			else operation = "mov_rcx,rax\nmov_rax,rbx\nshr_rax,cl\n";
		}
		else if(ARMV7L == Architecture)
		{
			if(is_signed) operation = "ARITH_RIGHT R1 R0 R0 SHIFT AUX_ALWAYS\n";
			else operation = "RIGHT R1 R0 R0 SHIFT AUX_ALWAYS\n";
		}
		else if(AARCH64 == Architecture)
		{
			if(is_signed) operation = "ARITH_RSHIFT_X0_X1_X0\n";
			else operation = "LOGICAL_RSHIFT_X0_X1_X0\n";
		}
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
		{
			if(is_signed) operation = "rd_a0 rs1_a1 rs2_a0 sra\n";
			else operation = "rd_a0 rs1_a1 rs2_a0 srl\n";
		}
	}
	else if(match("&=", operator))
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) operation = "AND R0 R0 R1\n";
		else if(X86 == Architecture) operation = "and_eax,ebx\n";
		else if(AMD64 == Architecture) operation = "and_rax,rbx\n";
		else if(ARMV7L == Architecture) operation = "NO_SHIFT R0 R0 AND R1 ARITH2_ALWAYS\n";
		else if(AARCH64 == Architecture) operation = "AND_X0_X1_X0\n";
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) operation = "rd_a0 rs1_a1 rs2_a0 and\n";
	}
	else if(match("^=", operator))
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) operation = "XOR R0 R0 R1\n";
		else if(X86 == Architecture) operation = "xor_eax,ebx\n";
		else if(AMD64 == Architecture) operation = "xor_rax,rbx\n";
		else if(ARMV7L == Architecture) operation = "'0' R0 R0 XOR R1 ARITH2_ALWAYS\n";
		else if(AARCH64 == Architecture) operation = "XOR_X0_X1_X0\n";
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) operation = "rd_a0 rs1_a1 rs2_a0 xor\n";
	}
	else if(match("|=", operator))
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) operation = "OR R0 R0 R1\n";
		else if(X86 == Architecture) operation = "or_eax,ebx\n";
		else if(AMD64 == Architecture) operation = "or_rax,rbx\n";
		else if(ARMV7L == Architecture) operation = "NO_SHIFT R0 R0 OR R1 AUX_ALWAYS\n";
		else if(AARCH64 == Architecture) operation = "OR_X0_X1_X0\n";
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) operation = "rd_a0 rs1_a1 rs2_a0 or\n";
	}
	else
	{
		fputs("Found illegal compound assignment operator: ", stderr);
		fputs(operator, stderr);
		fputc('\n', stderr);
		exit(EXIT_FAILURE);
	}
	return operation;
}


void expression(void)
{
	bitwise_expr();
	if(match("=", global_token->s))
	{
		char* store = "";
		if(match("]", global_token->prev->s))
		{
			store = store_value(current_target->type->size);
		}
		else
		{
			store = store_value(current_target->size);
		}

		common_recursion(expression);
		emit_out(store);
		current_target = integer;
	}
	else if(is_compound_assignment(global_token->s))
	{
		maybe_bootstrap_error("compound operator");
		char* load = "";
		char* operation = "";
		char* store = "";
		struct type* last_type = current_target;
		int is_array_indexed = match("]", global_token->prev->s);

		if(!is_array_indexed || !match("char*", current_target->name))
		{
			if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) load = "LOAD R1 R1 0\n";
			else if(X86 == Architecture) load = "mov_ebx,[ebx]\n";
			else if(AMD64 == Architecture) load = "mov_rbx,[rbx]\n";
			else if(ARMV7L == Architecture) load = "!0 R1 LOAD32 R1 MEMORY\n";
			else if(AARCH64 == Architecture) load = "DEREF_X1\n";
			else if(RISCV32 == Architecture) load = "rd_a1 rs1_a1 lw\n";
			else if(RISCV64 == Architecture) load = "rd_a1 rs1_a1 ld\n";
		}
		else
		{
			if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) load = "LOAD8 R1 R1 0\n";
			else if(X86 == Architecture) load = "movsx_ebx,BYTE_PTR_[ebx]\n";
			else if(AMD64 == Architecture) load = "movsx_rbx,BYTE_PTR_[rbx]\n";
			else if(ARMV7L == Architecture) load = "LOADU8 R1 LOAD R1 MEMORY\n";
			else if(AARCH64 == Architecture) load = "DEREF_X1_BYTE\n";
			else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) load = "rd_a1 rs1_a1 lbu\n";
		}

		char *operator = global_token->s;

		if(is_array_indexed)
		{
			store = store_value(current_target->type->size);
		}
		else
		{
			store = store_value(current_target->size);
		}

		int should_apply_pointer_arithmetic = type_is_pointer(current_target) && !is_array_indexed;
		/* We need this before it's changed by the following expression  */
		int object_size = current_target->type->size;

		common_recursion(expression);
		current_target = promote_type(current_target, last_type);
		emit_push(REGISTER_ONE, "compound operator");
		emit_out(load);

		if(should_apply_pointer_arithmetic) {
			multiply_by_object_size(object_size);
		}

		operation = compound_operation(operator, current_target->is_signed);
		emit_out(operation);
		emit_pop(REGISTER_ONE, "compound operator");
		emit_out(store);
		current_target = integer;
	}
}


int iskeywordp(char* s)
{
	if(match("auto", s)) return TRUE;
	if(match("_Bool", s)) return TRUE;
	if(match("break", s)) return TRUE;
	if(match("case", s)) return TRUE;
	if(match("char", s)) return TRUE;
	if(match("const", s)) return TRUE;
	if(match("continue", s)) return TRUE;
	if(match("default", s)) return TRUE;
	if(match("do", s)) return TRUE;
	if(match("double", s)) return TRUE;
	if(match("else", s)) return TRUE;
	if(match("enum", s)) return TRUE;
	if(match("extern", s)) return TRUE;
	if(match("float", s)) return TRUE;
	if(match("for", s)) return TRUE;
	if(match("goto", s)) return TRUE;
	if(match("if", s)) return TRUE;
	if(match("int", s)) return TRUE;
	if(match("long", s)) return TRUE;
	if(match("_Noreturn", s)) return TRUE;
	if(match("register", s)) return TRUE;
	if(match("restrict", s)) return TRUE;
	if(match("return", s)) return TRUE;
	if(match("short", s)) return TRUE;
	if(match("signed", s)) return TRUE;
	if(match("sizeof", s)) return TRUE;
	if(match("static", s)) return TRUE;
	if(match("struct", s)) return TRUE;
	if(match("switch", s)) return TRUE;
	if(match("typedef", s)) return TRUE;
	if(match("union", s)) return TRUE;
	if(match("unsigned", s)) return TRUE;
	if(match("void", s)) return TRUE;
	if(match("volatile", s)) return TRUE;
	if(match("while", s)) return TRUE;
	return FALSE;
}

/* Similar to integer division a / b but rounds up */
unsigned ceil_div(unsigned a, unsigned b)
{
	return (a + b - 1) / b;
}

int locals_depth;
/* Process local variable */
void collect_local(void)
{
	if(NULL != break_target_func)
	{
		maybe_bootstrap_error("Variable inside loop");
	}

	struct type* type_size = type_name();
	if(type_size->size == NO_STRUCT_DEFINITION)
	{
		line_error();
		fputs("Usage of struct '", stderr);
		fputs(type_size->name, stderr);
		fputs("' without definition (forward declaration only).\n", stderr);
		exit(EXIT_FAILURE);
	}

	/* Declarations do not have the same pointer level so we'll need to find the actual type */
	struct type* base_type = type_size->type->type;
	struct type* current_type = type_size;

	require(NULL != global_token, "Received EOF while collecting locals\n");

	struct token_list* list_to_append_to = function->locals;
	struct token_list* a;
	unsigned struct_depth_adjustment;
	char* name;
	int function_depth_offset = 0;

	do
	{
		if(global_token->s[0] == '(')
		{
			name = parse_function_pointer();
			current_type = function_pointer;
		}
		else
		{
			name = global_token->s;
			require_extra_token();
		}

		a = sym_declare(name, current_type, list_to_append_to, TLO_LOCAL);
		list_to_append_to = a;

		require(!in_set(name[0], "[{(<=>)}]|&!^%;:'\""), "forbidden character in local variable name\n");
		require(!iskeywordp(name), "You are not allowed to use a keyword as a local variable name\n");

		emit_out("# Defining local ");
		emit_out(name);
		emit_out("\n");

		a->array_modifier = 1;
		if(match("[", global_token->s))
		{
			maybe_bootstrap_error("array on the stack");

			a->options = a->options | TLO_LOCAL_ARRAY;

			require_extra_token();

			a->array_modifier = constant_expression();
			if(a->array_modifier == 0)
			{
				line_error();
				fputs("Size zero is not supported for arrays on the stack\n", stderr);
				exit(EXIT_FAILURE);
			}
			else if(a->array_modifier < 0)
			{
				line_error();
				fputs("Negative values are not supported for arrays on the stack\n", stderr);
				exit(EXIT_FAILURE);
			}
			else if(a->array_modifier > 0x100000)
			{
				line_error();
				fputs("M2-Planet is very inefficient so you probably don't want to allocate over 1MB onto the stack\n", stderr);
				exit(EXIT_FAILURE);
			}

			require_match("ERROR in collect_local\nMissing ] after local array size\n", "]");
		}

		if(NULL != function->locals)
		{
			function_depth_offset = function->locals->depth;
		}

		struct_depth_adjustment = ceil_div(a->type->size * a->array_modifier, register_size) * register_size;
		if(stack_direction == STACK_DIRECTION_PLUS)
		{
			a->depth = function_depth_offset + struct_depth_adjustment;
		}
		else
		{
			a->depth = function_depth_offset - struct_depth_adjustment;
		}
		locals_depth = locals_depth + struct_depth_adjustment;

		function->locals = a;

		if(match("=", global_token->s))
		{
			if(a->array_modifier != 1)
			{
				line_error();
				fputs("Stack arrays can not be initialized.\n", stderr);
				exit(EXIT_FAILURE);
			}

			require_extra_token();
			expression();

			load_address_of_variable_into_register(REGISTER_ONE, name);
			/* Store value of REGISTER_ZERO in REGISTER_ONE deref.
			 * The result of expression() will be in REGISTER_ZERO. */
			emit_out(store_value(type_size->size));
		}

		if(global_token->s[0] == ',')
		{
			maybe_bootstrap_error("multiple variables per statement");

			require_extra_token();

			current_type = base_type;
			while(global_token->s[0] == '*')
			{
				current_type = current_type->indirect;

				require_extra_token();
			}
		}
	}
	while(global_token->s[0] != ';');

	require_match("ERROR in collect_local\nMissing ;\n", ";");
}

void statement(void);

/* Evaluate if statements */
void process_if(void)
{
	char* number_string = int2str(current_count, 10, TRUE);
	current_count = current_count + 1;

	char* unique_id = create_unique_id("", function->s, number_string);

	emit_out("# IF_");
	emit_out(unique_id);
	emit_out("\n");

	global_token = global_token->next;
	require_match("ERROR in process_if\nMISSING (\n", "(");
	expression();

	emit_jump_if_zero(REGISTER_ZERO, "ELSE_", unique_id, "Jump to else");

	require_match("ERROR in process_if\nMISSING )\n", ")");
	statement();
	require(NULL != global_token, "Reached EOF inside of function\n");

	int has_else = match("else", global_token->s);
	if(has_else)
	{
		emit_unconditional_jump("_END_IF_", unique_id, "Else statement");
	}

	emit_label("ELSE_", unique_id);

	if(has_else)
	{
		require_extra_token();
		statement();
		require_token();
	}
	emit_label("_END_IF_", unique_id);
}

void process_case(void)
{
process_case_iter:
	require(NULL != global_token, "incomplete case statement\n");
	if(match("case", global_token->s)) return;
	if(match("default", global_token->s)) return;
	if(global_token->s[0] == '}') return;

	if(match("break", global_token->s))
	{
		statement();
		require(NULL != global_token, "incomplete case statement\n");
	}
	else
	{
		statement();
		require(NULL != global_token, "incomplete case statement\n");
		goto process_case_iter;
	}
}

void process_switch(void)
{
	maybe_bootstrap_error("switch/case statements");
	struct token_list* nested_locals = break_frame;
	char* nested_break_head = break_target_head;
	char* nested_break_func = break_target_func;
	char* nested_break_num = break_target_num;
	char* nested_continue_head = continue_target_head;

	char* number_string = int2str(current_count, 10, TRUE);
	current_count = current_count + 1;

	break_target_head = "_SWITCH_END_";
	continue_target_head = NULL; /* don't allow continue in switch statements */
	break_target_num = number_string;
	break_frame = function->locals;
	break_target_func = function->s;

	char* unique_id = create_unique_id("", function->s, number_string);

	emit_out("# switch_");
	emit_out(unique_id);
	emit_out("\n");

	/* get what we are casing on */
	global_token = global_token->next;
	require_match("ERROR in process_switch\nMISSING (\n", "(");
	expression();
	require_match("ERROR in process_switch\nMISSING )\n", ")");

	/* Put the value in R1 as it is currently in R0 */
	emit_move(REGISTER_ONE, REGISTER_ZERO, "process switch");

	emit_unconditional_jump("_SWITCH_TABLE_", unique_id, "Jump to the switch table");

	int has_default = FALSE;

	/* must be switch (exp) {$STATEMENTS}; form */
	require_match("ERROR in process_switch\nMISSING {\n", "{");
	struct case_list* backtrack = NULL;
	int value;
process_switch_iter:
	require(NULL != global_token, "incomplete switch statement\n");
	if(match("case", global_token->s))
	{
		require_extra_token();
		value = constant_expression();
		require_token();
		if(':' == global_token->s[0])
		{
			struct case_list* c = calloc(1, sizeof(struct case_list));
			c->next = backtrack;
			c->value = int2str(value, 10, FALSE);
			backtrack = c;
			emit_out(":_SWITCH_CASE_");
			emit_out(c->value);
			emit_out("_");
			emit_out(unique_id);
			emit_out("\n");
			require_extra_token();
			process_case();
			require_token();
		}
		else line_error();
		goto process_switch_iter;
	}
	else if(match("default", global_token->s))
	{ /* because of how M2-Planet treats labels */
		has_default = TRUE;
		require_extra_token();
		require_match("ERROR in process_switch\nMISSING : after default\n", ":");
		emit_label("_SWITCH_DEFAULT_", unique_id);

		require(NULL != global_token, "recieved EOF before switch closing }\n");
		/* collect statements until } */
		while(!match("}", global_token->s))
		{
			statement();
			require(NULL != global_token, "recieved EOF before switch closing }\n");
		}

		emit_unconditional_jump("_SWITCH_END_", unique_id, "jump over the switch table");
	}

	/* Switch statements must end with } */
	require_match("ERROR in process_switch\nMISSING }\n", "}");

	/* create the table */
	emit_label("_SWITCH_TABLE_", unique_id);

	char* buf;
	struct case_list* hold;
	while(NULL != backtrack)
	{
		/* put case value in R0 as the switch (value) is in R1 */
		emit_load_immediate(REGISTER_ZERO, strtoint(backtrack->value), "Load case value");
		hold = backtrack->next;

		buf = concat_strings3(backtrack->value, "_", unique_id);

		emit_jump_if_equal(REGISTER_ZERO, REGISTER_ONE, "_SWITCH_CASE_", buf, "Jump to case if equal");

		free(backtrack);
		backtrack = hold;
	}

	if(has_default)
	{
		emit_unconditional_jump("_SWITCH_DEFAULT_", unique_id, "Default to default:");
	}

	/* put the exit of the switch */
	emit_label("_SWITCH_END_", unique_id);

	break_target_head = nested_break_head;
	break_target_func = nested_break_func;
	break_target_num = nested_break_num;
	continue_target_head = nested_continue_head;
	break_frame = nested_locals;
}

void process_for(void)
{
	struct token_list* nested_locals = break_frame;
	char* nested_break_head = break_target_head;
	char* nested_break_func = break_target_func;
	char* nested_break_num = break_target_num;
	char* nested_continue_head = continue_target_head;

	char* number_string = int2str(current_count, 10, TRUE);
	current_count = current_count + 1;

	break_target_head = "FOR_END_";
	continue_target_head = "FOR_ITER_";
	break_target_num = number_string;
	break_frame = function->locals;
	break_target_func = function->s;

	char* unique_id = create_unique_id("", function->s, number_string);

	emit_out("# FOR_initialization_");
	emit_out(unique_id);
	emit_out("\n");

	require_extra_token();

	require_match("ERROR in process_for\nMISSING (\n", "(");
	/* fallible_type_name moves the global token if non-NULL */
	struct token_list* current = global_token;
	if (fallible_type_name() != NULL)
	{
		global_token = current;
		collect_local();
	}
	else if(!match(";", global_token->s))
	{
		expression();
		require_match("ERROR in process_for\nMISSING ;1\n", ";");
	}
	else
	{
		require_match("ERROR in process_for\nMISSING ;3\n", ";");
	}

	emit_label("FOR_", unique_id);

	expression();

	emit_jump_if_zero(REGISTER_ZERO, "FOR_END_", unique_id, "Jump to end");

	emit_unconditional_jump("FOR_THEN_", unique_id, "Go to body");

	emit_label("FOR_ITER_", unique_id);

	require_match("ERROR in process_for\nMISSING ;2\n", ";");
	expression();

	emit_unconditional_jump("FOR_", unique_id, "Check conditional");

	emit_label("FOR_THEN_", unique_id);

	require_match("ERROR in process_for\nMISSING )\n", ")");
	statement();
	require(NULL != global_token, "Reached EOF inside of function\n");

	emit_unconditional_jump("FOR_ITER_", unique_id, "Repeat iteration");

	emit_label("FOR_END_", unique_id);

	break_target_head = nested_break_head;
	break_target_func = nested_break_func;
	break_target_num = nested_break_num;
	continue_target_head = nested_continue_head;
	break_frame = nested_locals;
}

/* Process Assembly statements */
void process_asm(void)
{
	global_token = global_token->next;
	require_match("ERROR in process_asm\nMISSING (\n", "(");
	while('"' == global_token->s[0])
	{
		emit_out((global_token->s + 1));
		emit_out("\n");
		require_extra_token();
	}
	require_match("ERROR in process_asm\nMISSING )\n", ")");
	require_match("ERROR in process_asm\nMISSING ;\n", ";");
}

/* Process do while loops */
void process_do(void)
{
	struct token_list* nested_locals = break_frame;
	char* nested_break_head = break_target_head;
	char* nested_break_func = break_target_func;
	char* nested_break_num = break_target_num;
	char* nested_continue_head = continue_target_head;

	char* number_string = int2str(current_count, 10, TRUE);
	current_count = current_count + 1;

	char* unique_id = create_unique_id("", function->s, number_string);

	break_target_head = "DO_END_";
	continue_target_head = "DO_TEST_";
	break_target_num = number_string;
	break_frame = function->locals;
	break_target_func = function->s;

	emit_label("DO_", unique_id);

	require_extra_token();
	statement();
	require_token();

	emit_label("DO_TEST_", unique_id);

	require_match("ERROR in process_do\nMISSING while\n", "while");
	require_match("ERROR in process_do\nMISSING (\n", "(");
	expression();
	require_match("ERROR in process_do\nMISSING )\n", ")");
	require_match("ERROR in process_do\nMISSING ;\n", ";");

	emit_jump_if_not_zero(REGISTER_ZERO, "DO_", unique_id, "Rerun loop");

	emit_label("DO_END_", unique_id);

	break_frame = nested_locals;
	break_target_head = nested_break_head;
	break_target_func = nested_break_func;
	break_target_num = nested_break_num;
	continue_target_head = nested_continue_head;
}


/* Process while loops */
void process_while(void)
{
	struct token_list* nested_locals = break_frame;
	char* nested_break_head = break_target_head;
	char* nested_break_func = break_target_func;
	char* nested_break_num = break_target_num;
	char* nested_continue_head = continue_target_head;

	char* number_string = int2str(current_count, 10, TRUE);
	current_count = current_count + 1;

	char* unique_id = create_unique_id("", function->s, number_string);

	break_target_head = "END_WHILE_";
	continue_target_head = "WHILE_";
	break_target_num = number_string;
	break_frame = function->locals;
	break_target_func = function->s;

	emit_label("WHILE_", unique_id);

	global_token = global_token->next;
	require_match("ERROR in process_while\nMISSING (\n", "(");
	expression();

	emit_jump_if_zero(REGISTER_ZERO, "END_WHILE_", unique_id, "Jump to end");

	emit_out("# THEN_while_");
	emit_out(unique_id);
	emit_out("\n");

	require_match("ERROR in process_while\nMISSING )\n", ")");
	statement();
	require(NULL != global_token, "Reached EOF inside of function\n");

	emit_unconditional_jump("WHILE_", unique_id, "Repeat loop");

	emit_label("END_WHILE_", unique_id);

	break_target_head = nested_break_head;
	break_target_func = nested_break_func;
	break_target_num = nested_break_num;
	continue_target_head = nested_continue_head;
	break_frame = nested_locals;
}

char* function_locals_cleanup_string;
/* Ensure that functions return */
void return_result(void)
{
	require_extra_token();
	if(global_token->s[0] != ';') expression();

	require_match("ERROR in return_result\nMISSING ;\n", ";");

	emit_out(function_locals_cleanup_string);
	emit_return();
}

void process_break(void)
{
	if(NULL == break_target_head)
	{
		line_error();
		fputs("Not inside of a loop or case statement\n", stderr);
		exit(EXIT_FAILURE);
	}

	require_extra_token();

	char* break_target = concat_strings4(break_target_head, break_target_func, "_", break_target_num);
	emit_unconditional_jump("", break_target, "Break statement");

	require_match("ERROR in break statement\nMissing ;\n", ";");
}

void process_continue(void)
{
	if(NULL == continue_target_head)
	{
		line_error();
		fputs("Not inside of a loop\n", stderr);
		exit(EXIT_FAILURE);
	}
	require_extra_token();

	char* continue_target = concat_strings4(continue_target_head, break_target_func, "_", break_target_num);

	emit_unconditional_jump("", continue_target, "Continue statement");

	require_match("ERROR in continue statement\nMissing ;\n", ";");
}

void recursive_statement(void)
{
	require_extra_token();
	struct token_list* frame = function->locals;

	while(!match("}", global_token->s))
	{
		statement();
		require(NULL != global_token, "Received EOF in recursive statement prior to }\n");
	}
	global_token = global_token->next;

	function->locals = frame;
}

/* Variables inside loops are currently just global variables */
void process_static_variable(void)
{
	maybe_bootstrap_error("static local variable");

	struct type* type_size = type_name();
	char* name = global_token->s;

	struct static_variable_list* variable = calloc(1, sizeof(struct static_variable_list));
	variable->next = function_static_variables_list;
	function_static_variables_list = variable;
	variable->local_variable_name = name;

	char* new_name = concat_strings3(function->s, "_", name);

	variable->global_variable = sym_declare(new_name, type_size, NULL, TLO_STATIC);
	require_extra_token();

	declare_global_variable(type_size, variable->global_variable);

	require_match("Expected ; after static variable declarations\n", ";");
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

struct type* lookup_primitive_type(void);
void statement(void)
{
	require(NULL != global_token, "expected a C statement but received EOF\n");
	/* Always an integer until told otherwise */
	current_target = integer;

	struct token_list* current_token = global_token;

	struct token_list* next_token = global_token->next;
	int is_label = FALSE;
	if(next_token != NULL)
	{
		if(next_token->s[0] == ':')
		{
			is_label = TRUE;
		}
	}

	if(global_token->s[0] == '{')
	{
		recursive_statement();
	}
	else if(is_label)
	{
		emit_out(":");
		emit_out(global_token->s);
		emit_out("\t#C goto label\n");
		global_token = global_token->next;
		require_extra_token();
	}
	else if((NULL != lookup_primitive_type()) ||
	          match("enum", global_token->s) ||
	          match("struct", global_token->s) ||
	          match("union", global_token->s) ||
	          match("const", global_token->s))
	{
		/* Multi token lookup may move the global_token, but collect_local does a lookup of its own. */
		global_token = current_token;
		collect_local();
	}
	else if(match("if", global_token->s))
	{
		process_if();
	}
	else if(match("switch", global_token->s))
	{
		process_switch();
	}
	else if(match("do", global_token->s))
	{
		process_do();
	}
	else if(match("while", global_token->s))
	{
		process_while();
	}
	else if(match("for", global_token->s))
	{
		process_for();
	}
	else if(match("asm", global_token->s))
	{
		process_asm();
	}
	else if(match("goto", global_token->s))
	{
		require_extra_token();
		emit_unconditional_jump("", global_token->s, "Goto");
		require_extra_token();
		require_match("ERROR in statement\nMissing ;\n", ";");
	}
	else if(match("return", global_token->s))
	{
		return_result();
	}
	else if(match("break", global_token->s))
	{
		process_break();
	}
	else if(match("continue", global_token->s))
	{
		process_continue();
	}
	else if(match("static", global_token->s))
	{
		require_extra_token();

		process_static_variable();
	}
	else
	{
		expression();
		require_match("ERROR in statement\nMISSING ;\n", ";");
	}
}

/* Collect function arguments */
void collect_arguments(void)
{
	require_extra_token();
	struct type* type_size;
	struct token_list* a;

	while(!match(")", global_token->s))
	{
		if(global_token->s[0] == '.')
		{
			/* Periods can only be in the argument list as a variadic parameter
			 * so if there is a period it's part of a variadic parameter */
			require_match("Invalid token found in variadic arguments token", ".");
			require_match("Invalid token found in variadic arguments token", ".");
			require_match("Invalid token found in variadic arguments token", ".");

			maybe_bootstrap_error("variadic functions");
			break;
		}

		type_size = type_name();
		require(NULL != global_token, "Received EOF when attempting to collect arguments\n");
		require(NULL != type_size, "Must have non-null type\n");
		if(global_token->s[0] == ')')
		{
			/* foo(int,char,void) doesn't need anything done */
			continue;
		}
		else if(global_token->s[0] != ',')
		{
			/* deal with foo(int a, char b) */
			require(!in_set(global_token->s[0], "[{(<=>)}]|&!^%;:'\""), "forbidden character in argument variable name\n");
			require(!iskeywordp(global_token->s), "You are not allowed to use a keyword as a argument variable name\n");
			a = sym_declare(global_token->s, type_size, function->arguments, TLO_ARGUMENT);
			if(NULL == function->arguments)
			{
				if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) a->depth = 0;
				else if(X86 == Architecture) a->depth = -4;
				else if(AMD64 == Architecture) a->depth = -8;
				else if(ARMV7L == Architecture) a->depth = 4;
				else if(AARCH64 == Architecture) a->depth = register_size;
				else if(RISCV32 == Architecture) a->depth = -4;
				else if(RISCV64 == Architecture) a->depth = -8;
			}
			else
			{
				if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) a->depth = function->arguments->depth + register_size;
				else if(X86 == Architecture) a->depth = function->arguments->depth - register_size;
				else if(AMD64 == Architecture) a->depth = function->arguments->depth - register_size;
				else if(ARMV7L == Architecture) a->depth = function->arguments->depth + register_size;
				else if(AARCH64 == Architecture) a->depth = function->arguments->depth + register_size;
				else if(RISCV32 == Architecture) a->depth = function->arguments->depth - register_size;
				else if(RISCV64 == Architecture) a->depth = function->arguments->depth - register_size;
			}

			require_extra_token();
			function->arguments = a;

			while(global_token->s[0] == '[')
			{
				require_extra_token();

				if(global_token->s[0] != ']')
				{
					/* Throw away the result since we don't use it anyway */
					constant_expression();
				}

				require_match("Invalid token received in argument array, expected ']'.\n", "]");
				a->type = a->type->indirect;
			}
		}

		/* ignore trailing comma (needed for foo(bar(), 1); expressions*/
		if(global_token->s[0] == ',')
		{
			require_extra_token();
		}

		require(NULL != global_token, "Argument list never completed\n");
	}
	require_extra_token();
}

void declare_function(void)
{
	current_count = 0;
	function = sym_declare(global_token->prev->s, NULL, global_function_list, TLO_FUNCTION);

	/* allow previously defined functions to be looked up */
	global_function_list = function;
	if((KNIGHT_NATIVE == Architecture) && match("main", function->s))
	{
		require_match("Impossible error ( vanished\n", "(");
		require_match("Reality ERROR (USING KNIGHT-NATIVE)\nHardware does not support arguments\nthus neither can main on this architecture\ntry tape_01 and tape_02 instead\n", ")");
	}
	else collect_arguments();

	require(NULL != global_token, "Function definitions either need to be prototypes or full\n");
	/* If just a prototype don't waste time */
	if(global_token->s[0] == ';') require_extra_token();
	else
	{
		emit_out("# Defining function ");
		emit_out(function->s);
		emit_out("\n");
		emit_label("FUNCTION_", function->s);

		locals_depth = 0;

		char* allocate_locals_string = calloc(MAX_STRING, sizeof(char));

		/* Save the current location of the stack pointer. */
		emit_out(allocate_locals_string);

		char* stack_reserve_string = calloc(MAX_STRING, sizeof(char));
		emit_out(stack_reserve_string);

		function_locals_cleanup_string = calloc(MAX_STRING, sizeof(char));

		/* If we add any statics we don't want them globally available */
		function_static_variables_list = NULL;
		statement();
		/* Just to be sure this doesn't escape the function somehow. */
		function_static_variables_list = NULL;

		if(locals_depth != 0)
		{
			reset_emit_string();
			if(stack_direction == STACK_DIRECTION_PLUS)
			{
				write_add_immediate(REGISTER_STACK, locals_depth, "Reserve stack");
			}
			else
			{
				write_sub_immediate(REGISTER_STACK, locals_depth, "Reserve stack");
			}
			copy_string(stack_reserve_string, emit_string, MAX_STRING);

			/* Only write this if there are locals. Otherwise leave empty. */
			reset_emit_string();
			write_move(REGISTER_LOCALS, REGISTER_STACK, "Set locals pointer");
			copy_string(allocate_locals_string, emit_string, MAX_STRING);

			reset_emit_string();
			write_move(REGISTER_STACK, REGISTER_LOCALS, "Undo local variables");
			copy_string(function_locals_cleanup_string, emit_string, MAX_STRING);
		}

		/* C99 5.1.2.2.3 Program termination
		 * [..] reaching the } that terminates the main function returns a value of 0.
		 * */
		int is_main = match(function->s, "main");
		if (!match(return_instruction, output_list->s))
		{
			if(is_main) emit_load_immediate(REGISTER_ZERO, 0, "declare function");
			emit_out(function_locals_cleanup_string);
			emit_return();
		}

		emit_out("\n");
	}
}

struct type* typedef_function_pointer(void)
{
	char* name = parse_function_pointer();

	if(match(name, "FUNCTION"))
	{
		/* Don't create unnecessary duplicates of built-in types */
		return function_pointer;
	}

	return new_function_pointer_typedef(name);
}

struct type* global_typedef(void)
{
	require_extra_token(); /* skip 'typedef' */

	struct type* type_size = type_name();
	require(NULL != global_token, "Received EOF while reading typedef\n");

	if(global_token->s[0] == '(')
	{
		typedef_function_pointer();
	}
	else
	{
		type_size = mirror_type(type_size);
		global_token = global_token->next;
	}

	require_match("ERROR in typedef statement\nMissing ;\n", ";");

	return type_size;
}

void global_variable_header(char* name)
{
	globals_list = emit(":GLOBAL_", globals_list);
	globals_list = emit(name, globals_list);
	globals_list = emit("\n", globals_list);
}

void global_variable_zero_initialize(int size)
{
	unsigned i = ceil_div(size, register_size);
	while(i != 0)
	{
		/* NULLs are defined in the *_defs.M1 files to be register_size.
		 * This will round objects up to a multiple of register_size, but
		 * it will not have a negative effect.
		 * */
		globals_list = emit("NULL\n", globals_list);
		i = i - 1;
	}
}

void global_value_output(int value, int size)
{
	if(size == 1)
	{
		globals_list = emit(integer_to_raw_byte_string(value), globals_list);
	}
	else if(size == 2)
	{
		if(value < 0 && Architecture != RISCV64 && Architecture != RISCV32)
		{
			globals_list = emit("@", globals_list);
		}
		else
		{
			globals_list = emit("$", globals_list);
		}
		globals_list = emit(int2str(value, 10, FALSE), globals_list);
		globals_list = emit(" ", globals_list);
	}
	else if(size >= 4)
	{
		globals_list = emit("%", globals_list);
		globals_list = emit(int2str(value, 10, FALSE), globals_list);
		globals_list = emit(" ", globals_list);

		if(size == 8)
		{
			if(value < 0)
			{
				globals_list = emit("%0xffffffff ", globals_list);
			}
			else
			{
				globals_list = emit("%0 ", globals_list);
			}
		}
	}

}

void global_pad_to_register_size(int bytes_written)
{
	int remaining = (bytes_written % register_size);
	if(remaining != 0)
	{
		int alignment_size = register_size - remaining;
		while(alignment_size != 0)
		{
			globals_list = emit("'00' ", globals_list);

			alignment_size = alignment_size - 1;
		}
	}

	globals_list = emit("\n", globals_list);
}

void global_struct_initializer_list(struct type* type_size);
void global_value_selection(struct type* type_size)
{
	if(type_is_pointer(type_size))
	{
		if(('"' == global_token->s[0]))
		{
			char* name = global_token->s + 1;
			globals_list = emit("&GLOBAL_", globals_list);
			globals_list = emit(name, globals_list);
			globals_list = emit("_contents ", globals_list);

			if(register_size == 8)
			{
				global_value_output(0, 4);
			}

			strings_list = emit(":GLOBAL_", strings_list);
			strings_list = emit(name, strings_list);
			strings_list = emit("_contents\n", strings_list);
			strings_list = emit(parse_string(global_token->s), strings_list);

			require_extra_token();
		}
		else if(match("0", global_token->s))
		{
			global_value_output(0, register_size);
			require_extra_token();
		}
		else if(global_token->s[0] == '&')
		{
			require_extra_token();

			char* name = global_token->s;
			struct token_list* lookup_token = sym_lookup(name, global_function_list);
			if(NULL != lookup_token)
			{
				globals_list = emit("&FUNCTION_", globals_list);
				globals_list = emit(name, globals_list);
				globals_list = emit(" ", globals_list);
			}
			else
			{
				lookup_token = sym_lookup(name, global_symbol_list);
				if(NULL != lookup_token)
				{
					globals_list = emit("&GLOBAL_", globals_list);
					globals_list = emit(name, globals_list);
					globals_list = emit(" ", globals_list);
				}
				else
				{
					line_error();
					fputs("Unable to find address of '", stderr);
					fputs(name, stderr);
					fputs("'.\n", stderr);
					exit(EXIT_FAILURE);
				}
			}
			if(register_size > 4)
			{
				globals_list = emit("%0 ", globals_list);
			}
			require_extra_token();
		}
		else
		{
			line_error();
			fputs("Invalid initializer for global struct pointer member.\n", stderr);
			exit(EXIT_FAILURE);
		}
	}
	else if(type_is_struct_or_union(type_size))
	{
		global_struct_initializer_list(type_size);
	}
	else
	{
		int value = constant_expression();
		global_value_output(value, type_size->size);
	}
}

void global_struct_initializer_list(struct type* type_size)
{
	require_match("Struct assignment initialization is invalid for globals.", "{");
	require(NULL != global_token, "EOF in global struct initialization");

	struct type* member = type_size->members;

	do
	{
		if(member == NULL)
		{
			line_error();
			fputs("Global struct initializer list has too many values.\n", stderr);
			exit(EXIT_FAILURE);
		}

		global_value_selection(member->type);

		member = member->members;

		if(global_token->s[0] == ',')
		{
			require_extra_token();
		}
	}
	while(global_token->s[0] != '}');

	while(member != NULL)
	{
		global_value_output(0, member->size);
		member = member->members;
	}

	globals_list = emit("\n", globals_list);

	require_match("Struct assignment initialization is invalid for globals.", "}");
	require(NULL != global_token, "EOF in global struct initialization");
}

int global_array_initializer_list(struct type* type_size, int array_modifier)
{
	int amount_of_elements = 0;

	do
	{
		if(amount_of_elements >= array_modifier && array_modifier != 0)
		{
			line_error();
			fputs("Too many elements in initializer list.", stderr);
			exit(EXIT_FAILURE);
		}

		global_value_selection(type_size);

		amount_of_elements = amount_of_elements + 1;

		if(global_token->s[0] == ',')
		{
			require_extra_token();
		}
	}
	while (global_token->s[0] != '}');

	require_extra_token();

	if(array_modifier == 0)
	{
		if(amount_of_elements == 0)
		{
			line_error();
			fputs("Array with initializer list can not have size zero.\n", stderr);
			exit(EXIT_FAILURE);
		}

		array_modifier = amount_of_elements;
	}

	int size;
	while(amount_of_elements < array_modifier)
	{
		size = type_size->size;
		if(size == 1 || size == 4 || size == 8)
		{
			global_value_output(0, size);
		}
		else
		{
			while(size > 0)
			{
				if(size >= 4)
				{
					global_value_output(0, 4);
					size = size - 4;
				}
				else
				{
					global_value_output(0, 1);
					size = size - 1;
				}
			}
			globals_list = emit("\n", globals_list);
		}

		amount_of_elements = amount_of_elements + 1;
	}


	return array_modifier;
}

int global_static_array(struct type* type_size, char* name)
{
	global_variable_header(name);

	if(global_token->s[0] == ';' || global_token->s[0] == ',')
	{
		global_variable_zero_initialize(type_size->size);
		return 0;
	}
	else if(global_token->s[0] == '=')
	{
		require_extra_token();

		global_value_selection(type_size);

		global_pad_to_register_size(type_size->size);
		return 0;
	}

	maybe_bootstrap_error("global array definitions");

	globals_list = emit("&GLOBAL_STORAGE_", globals_list);
	globals_list = emit(name, globals_list);
	if (AARCH64 == Architecture || AMD64 == Architecture || RISCV64 == Architecture)
	{
		globals_list = emit(" %0", globals_list);
	}
	globals_list = emit("\n:GLOBAL_STORAGE_", globals_list);
	globals_list = emit(name, globals_list);
	globals_list = emit("\n", globals_list);

	require_extra_token();

	int array_modifier = 0;
	int size = 0;
	if(global_token->s[0] != ']')
	{
		array_modifier = constant_expression();
		/* Make sure not negative */
		if(array_modifier < 0)
		{
			line_error();
			fputs("Negative values are not supported for allocated arrays\n", stderr);
			exit(EXIT_FAILURE);
		}

		/* length */
		size = array_modifier * type_size->size;

		if(size == 0)
		{
			line_error();
			fputs("Arrays with size of zero are not allowed.\n", stderr);
			exit(EXIT_FAILURE);
		}

		/* Stop bad states */
		if((size < 0) || (size > 0x100000))
		{
			line_error();
			fputs("M2-Planet is very inefficient so you probably don't want to allocate over 1MB into your binary for NULLs\n", stderr);
			exit(EXIT_FAILURE);
		}
	}

	/* Ensure properly closed */
	require_match("missing close bracket\n", "]");

	if(global_token->s[0] == '=')
	{
		require_extra_token();

		require_match("Missing { after = in global array", "{");

		array_modifier = global_array_initializer_list(type_size, array_modifier);

		global_pad_to_register_size(array_modifier * type_size->size);
	}
	else
	{
		if(size == 0)
		{
			line_error();
			fputs("Array without size must have initializer list.", stderr);
			exit(EXIT_FAILURE);
		}

		global_variable_zero_initialize(size);
	}

	return array_modifier;
}

void declare_global_variable(struct type* type_size, struct token_list* variable)
{
	variable->array_modifier = global_static_array(type_size, variable->s);
}

/*
 * program:
 *     declaration
 *     declaration program
 *
 * declaration:
 *     CONSTANT identifer value
 *     typedef identifer type;
 *     type-name identifier ;
 *     type-name identifier = value ;
 *     type-name identifier [ value ];
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
void program(void)
{
	function = NULL;
	Address_of = FALSE;
	struct type* type_size;
	char* name;
	struct type* base_type;
	struct type* current_type;

new_type:
	/* Deal with garbage input */
	if (NULL == global_token) return;
	require('#' != global_token->s[0], "unhandled macro directive\n");
	require(!match("\n", global_token->s), "unexpected newline token\n");

	/* Handle c typedef statements */
	if(match("typedef", global_token->s))
	{
		type_size = global_typedef();
		goto new_type;
	}

	/* declaration-specifiers can come in any order */
	while(match("inline", global_token->s)
			|| match("static", global_token->s)
			|| match("_Noreturn", global_token->s))
	{
		require_extra_token();
	}

	type_size = type_name();
	/* Deal with case of struct definitions */
	if(global_token->s[0] == ';')
	{
		global_token = global_token->next;
		goto new_type;
	}

	require(NULL != global_token->next, "Unterminated global\n");

	if(global_token->s[0] == '('
				|| global_token->next->s[0] == ','
				|| global_token->next->s[0] == ';'
				|| global_token->next->s[0] == '='
				|| global_token->next->s[0] == '[')
	{
		/* Declarations do not have the same pointer level so we'll need to find the actual type */
		base_type = type_size->type->type;
		current_type = type_size;

		do
		{
			if(global_token->s[0] == '(')
			{
				name = parse_function_pointer();
				current_type = function_pointer;
			}
			else
			{
				name = global_token->s;
				require_extra_token();
			}

			global_symbol_list = sym_declare(name, current_type, global_symbol_list, TLO_GLOBAL);
			declare_global_variable(current_type, global_symbol_list);

			if(global_token->s[0] == ',')
			{
				require_extra_token();

				current_type = base_type;
				while(global_token->s[0] == '*')
				{
					current_type = current_type->indirect;

					require_extra_token();
				}
			}
		}
		while(global_token->s[0] != ';');

		require_match("Expected ; after global declarations", ";");

		goto new_type;
	}

	require_extra_token();

	/* Deal with global functions */
	if(match("(", global_token->s))
	{
		declare_function();
		goto new_type;
	}

	/* Everything else is just an error */
	line_error();
	fputs("Received ", stderr);
	fputs(global_token->s, stderr);
	fputs(" in program\n", stderr);
	exit(EXIT_FAILURE);
}

void recursive_output(struct token_list* head, FILE* out)
{
	struct token_list* i = reverse_list(head);
	while(NULL != i)
	{
		fputs(i->s, out);
		i = i->next;
	}
}

void output_tokens(struct token_list *i, FILE* out)
{
	while(NULL != i)
	{
		fputs(i->s, out);
		fputs(" ", out);
		i = i->next;
	}
}
