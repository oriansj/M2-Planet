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

#include "cc.h"
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

void global_variable_definition(struct type*, char*);
void global_assignment(char*, struct type*);
int global_static_array(struct type*, char*);

struct token_list* emit(char *s, struct token_list* head)
{
	struct token_list* t = calloc(1, sizeof(struct token_list));
	require(NULL != t, "Exhausted memory while generating token to emit\n");
	t->next = head;
	t->s = s;
	return t;
}

void emit_out(char* s)
{
	output_list = emit(s, output_list);
}

char* register_from_string(int reg)
{
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		if(reg == REGISTER_ZERO) return "0";
		else if(reg == REGISTER_ONE) return "1";
		else if(reg == REGISTER_UNUSED1) return "2";
		else if(reg == REGISTER_UNUSED2) return "3";
		else if(reg == REGISTER_LOCALS) return "4";
		else if(reg == REGISTER_TEMP) return "13";
		else if(reg == REGISTER_BASE) return "14";
		else if(reg == REGISTER_STACK) return "15";
	}
	else if(X86 == Architecture)
	{
		if(reg == REGISTER_ZERO) return "eax";
		else if(reg == REGISTER_ONE) return "ebx";
		else if(reg == REGISTER_TEMP) return "edi";
		else if(reg == REGISTER_BASE) return "ebp";
		else if(reg == REGISTER_STACK) return "esp";
		else if(reg == REGISTER_LOCALS) return "esi";
		else if(reg == REGISTER_UNUSED1) return "ecx";
		else if(reg == REGISTER_UNUSED2) return "edx";
	}
	else if(AMD64 == Architecture)
	{
		if(reg == REGISTER_ZERO) return "rax";
		else if(reg == REGISTER_ONE) return "rbx";
		else if(reg == REGISTER_TEMP) return "rdi";
		else if(reg == REGISTER_BASE) return "rbp";
		else if(reg == REGISTER_STACK) return "rsp";
		else if(reg == REGISTER_LOCALS) return "rsi";
		else if(reg == REGISTER_UNUSED1) return "rcx";
		else if(reg == REGISTER_UNUSED2) return "rdx";
	}
	else if(ARMV7L == Architecture)
	{
		if(reg == REGISTER_ZERO) return "R0";
		else if(reg == REGISTER_ONE) return "R1";
		else if(reg == REGISTER_LOCALS) return "R2";
		else if(reg == REGISTER_UNUSED1) return "R3";
		else if(reg == REGISTER_UNUSED2) return "R4";
		else if(reg == REGISTER_TEMP) return "R11";
		else if(reg == REGISTER_BASE) return "BP";
		else if(reg == REGISTER_RETURN) return "LR";
		else if(reg == REGISTER_STACK) return "SP";
	}
	else if(AARCH64 == Architecture)
	{
		if(reg == REGISTER_ZERO) return "X0";
		else if(reg == REGISTER_ONE) return "X1";
		else if(reg == REGISTER_LOCALS) return "X2";
		else if(reg == REGISTER_UNUSED1) return "X3";
		else if(reg == REGISTER_UNUSED2) return "X4";
		else if(reg == REGISTER_TEMP) return "X16";
		else if(reg == REGISTER_BASE) return "BP";
		else if(reg == REGISTER_RETURN) return "LR";
		else if(reg == REGISTER_STACK) return "SP";
	}
	else if(RISCV32 == Architecture || RISCV64 == Architecture)
	{
		if(reg == REGISTER_ZERO) return "a0";
		else if(reg == REGISTER_ONE) return "a1";
		else if(reg == REGISTER_LOCALS) return "a2";
		else if(reg == REGISTER_UNUSED1) return "a3";
		else if(reg == REGISTER_UNUSED2) return "a4";
		else if(reg == REGISTER_TEMP) return "tp";
		else if(reg == REGISTER_BASE) return "fp";
		else if(reg == REGISTER_RETURN) return "ra";
		else if(reg == REGISTER_STACK) return "sp";
	}

	fputs("PROGRAMMING ERROR: Invalid register passed to register_from_string: '", stderr);
	fputs(int2str(reg, 10, FALSE), stderr);
	fputs("' for architecture '", stderr);
	fputs(int2str(Architecture, 10, FALSE), stderr);
	fputs("'\n.", stderr);
	exit(EXIT_FAILURE);
}

void emit_load_named_immediate(int reg, char* prefix, char* name, char* note)
{
	char* reg_name = register_from_string(reg);
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		emit_out("LOADR R");
		emit_out(reg_name);
		emit_out(" 4\nJUMP 4\n&");
		emit_out(prefix);
		emit_out(name);
	}
	else if(X86 == Architecture)
	{
		emit_out("mov_");
		emit_out(reg_name);
		emit_out(", &");
		emit_out(prefix);
		emit_out(name);
	}
	else if(AMD64 == Architecture)
	{
		emit_out("lea_");
		emit_out(reg_name);
		emit_out(",[rip+DWORD] %");
		emit_out(prefix);
		emit_out(name);
	}
	else if(ARMV7L == Architecture)
	{
		emit_out("!0 ");
		emit_out(reg_name);
		emit_out(" LOAD32 R15 MEMORY\n~0 JUMP_ALWAYS\n&");
		emit_out(prefix);
		emit_out(name);
	}
	else if(AARCH64 == Architecture)
	{
		emit_out("LOAD_W");
		/* Normal register starts with X for 64bit wide
		 * but we need W. */
		emit_out(reg_name + 1);
		emit_out("_AHEAD\nSKIP_32_DATA\n&");
		emit_out(prefix);
		emit_out(name);
	}
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
	{
		emit_out("rd_");
		emit_out(reg_name);
		emit_out(" ~");
		emit_out(prefix);
		emit_out(name);
		emit_out(" auipc\n");

		emit_out("rd_");
		emit_out(reg_name);
		emit_out(" rs1_");
		emit_out(reg_name);
		emit_out(" !");
		emit_out(prefix);
		emit_out(name);
		emit_out(" addi");
	}


	if(note == NULL)
	{
		emit_out("\n");
	}
	else
	{
		emit_out(" # ");
		emit_out(note);
		emit_out("\n");
	}
}

void emit_load_immediate(int reg, int value, char* note)
{
	char* reg_name = register_from_string(reg);
	char* value_string = int2str(value, 10, TRUE);
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		if((32767 > value) && (value > -32768))
		{
			emit_out("LOADI R");
			emit_out(reg_name);
			emit_out(" ");
			emit_out(value_string);
		}
		else
		{
			emit_out("LOADR R");
			emit_out(reg_name);
			emit_out(" 4\nJUMP 4\n%");
			emit_out(int2str(value, 10, TRUE));
		}
	}
	else if(X86 == Architecture || AMD64 == Architecture)
	{
		if(value == 0)
		{
			/* This is the recommended way of zeroing a register on x86/amd64.
			 * xor eax, eax (32 bit registers) for both x86 and amd64 since it
			 * takes up a byte less and still zeros the register. */
			emit_out("xor_e");
			/* amd64 register starts with r but we need it to start with e */
			emit_out(reg_name + 1);
			emit_out(",e");
			emit_out(reg_name + 1);
		}
		else
		{
			emit_out("mov_");
			emit_out(reg_name);
			emit_out(", %");
			emit_out(value_string);
		}
	}
	else if(ARMV7L == Architecture)
	{
		if((127 >= value) && (value >= -128))
		{
			emit_out("!");
			emit_out(value_string);
			emit_out(" ");
			emit_out(reg_name);
			emit_out(" LOADI8_ALWAYS");
		}
		else
		{
			emit_out("!0 ");
			emit_out(reg_name);
			emit_out(" LOAD32 R15 MEMORY\n~0 JUMP_ALWAYS\n%");
			emit_out(value_string);
		}
	}
	else if(AARCH64 == Architecture)
	{
		if((value == 0) || (value == 1 && reg == 0))
		{
			emit_out("SET_");
			emit_out(reg_name);
			emit_out("_TO_");
			emit_out(value_string);
		}
		else
		{
			emit_out("LOAD_W");
			/* Normal register starts with X for 64bit wide
			 * but we need W. */
			emit_out(reg_name + 1);
			emit_out("_AHEAD\nSKIP_32_DATA\n%");
			emit_out(value_string);
		}
	}
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
	{
		if((2047 >= value) && (value >= -2048))
		{
			emit_out("rd_");
			emit_out(reg_name);
			emit_out(" !");
			emit_out(value_string);
			emit_out(" addi");
		}
		else if (0 == (value >> 30))
		{
			emit_out("rd_");
			emit_out(reg_name);
			emit_out(" ~");
			emit_out(value_string);
			emit_out(" lui\n");

			emit_out("rd_");
			emit_out(reg_name);
			emit_out(" rs1_");
			emit_out(reg_name);
			emit_out(" !");
			emit_out(value_string);
			emit_out(" addi");
		}
		else
		{
			int high = value >> 30;
			char* high_string = int2str(high, 10, TRUE);
			int low = ((value >> 30) << 30) ^ value;
			char* low_string = int2str(low, 10, TRUE);

			emit_out("rd_");
			emit_out(reg_name);
			emit_out(" ~");
			emit_out(high_string);
			emit_out(" lui\n");

			emit_out("rd_");
			emit_out(reg_name);
			emit_out(" rs1_");
			emit_out(reg_name);
			emit_out(" !");
			emit_out(high_string);
			emit_out(" addi\n");

			emit_out("rd_");
			emit_out(reg_name);
			emit_out(" rs1_");
			emit_out(reg_name);
			emit_out(" rs2_x30 slli\n");

			emit_out("rd_t1 ~");
			emit_out(low_string);
			emit_out(" lui\n");

			emit_out("rd_t1 rs1_t1 !");
			emit_out(low_string);
			emit_out(" addi\n");

			emit_out("rd_");
			emit_out(reg_name);
			emit_out(" rs1_");
			emit_out(reg_name);
			emit_out(" rs2_t1 or");
		}
	}


	if(note == NULL)
	{
		emit_out("\n");
	}
	else
	{
		emit_out(" # ");
		emit_out(note);
		emit_out("\n");
	}
}

/* Adds destination and source and places result in destination */
void emit_add(int destination_reg, int source_reg, char* note)
{
	char* destination_name = register_from_string(destination_reg);
	char* source_name = register_from_string(source_reg);

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		emit_out("ADD R");
		emit_out(destination_name);
		emit_out(" R");
		emit_out(destination_name);
		emit_out(" R");
		emit_out(source_name);
	}
	else if(X86 == Architecture || AMD64 == Architecture)
	{
		emit_out("add_");
		emit_out(destination_name);
		emit_out(",");
		emit_out(source_name);
	}
	else if(ARMV7L == Architecture)
	{
		emit_out("'0' ");
		emit_out(destination_name);
		emit_out(" ");
		emit_out(destination_name);
		emit_out(" ADD ");
		emit_out(source_name);
		emit_out(" ARITH2_ALWAYS");
	}
	else if(AARCH64 == Architecture)
	{
		emit_out("ADD_");
		emit_out(destination_name);
		emit_out("_");
		emit_out(source_name);
		emit_out("_");
		emit_out(destination_name);
	}
	else if(RISCV32 == Architecture || RISCV64 == Architecture)
	{
		emit_out("rd_");
		emit_out(destination_name);
		emit_out(" rs1_");
		emit_out(source_name);
		emit_out(" rs2_");
		emit_out(destination_name);
		emit_out(" add");
	}

	if(note == NULL)
	{
		emit_out("\n");
	}
	else
	{
		emit_out(" # ");
		emit_out(note);
		emit_out("\n");
	}
}

void emit_add_immediate(int reg, int value, char* note)
{
	emit_push(REGISTER_ONE, note);
	emit_load_immediate(REGISTER_ONE, value, note);
	emit_add(reg, REGISTER_ONE, note);
	emit_pop(REGISTER_ONE, note);
}

/* Subtracts destination and source and places result in destination */
void emit_sub(int destination_reg, int source_reg, char* note)
{
	char* destination_name = register_from_string(destination_reg);
	char* source_name = register_from_string(source_reg);

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		emit_out("SUB R");
		emit_out(destination_name);
		emit_out(" R");
		emit_out(destination_name);
		emit_out(" R");
		emit_out(source_name);
	}
	else if(X86 == Architecture || AMD64 == Architecture)
	{
		emit_out("sub_");
		emit_out(destination_name);
		emit_out(",");
		emit_out(source_name);
		emit_out("\n");
	}
	else if(ARMV7L == Architecture)
	{
		emit_out("'0' ");
		emit_out(destination_name);
		emit_out(" ");
		emit_out(destination_name);
		emit_out(" SUB ");
		emit_out(source_name);
		emit_out(" ARITH2_ALWAYS");
	}
	else if(AARCH64 == Architecture)
	{
		emit_out("SUB_");
		emit_out(destination_name);
		emit_out("_");
		emit_out(source_name);
		emit_out("_");
		emit_out(destination_name);
	}
	else if(RISCV32 == Architecture || RISCV64 == Architecture)
	{
		emit_out("rd_");
		emit_out(destination_name);
		emit_out(" rs1_");
		emit_out(destination_name);
		emit_out(" rs2_");
		emit_out(source_name);
		emit_out(" sub");
	}

	if(note == NULL)
	{
		emit_out("\n");
	}
	else
	{
		emit_out(" # ");
		emit_out(note);
		emit_out("\n");
	}
}

void emit_sub_immediate(int reg, int value, char* note)
{
	emit_push(REGISTER_ONE, note);
	emit_load_immediate(REGISTER_ONE, value, note);
	emit_sub(reg, REGISTER_ONE, note);
	emit_pop(REGISTER_ONE, note);
}

void emit_mul_into_register_zero(int reg, char* note)
{
	char* reg_name = register_from_string(reg);

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		emit_out("MULU R0 R");
		emit_out(reg_name);
		emit_out(" R0");
	}
	else if(X86 == Architecture || AMD64 == Architecture)
	{
		emit_out("mul_");
		emit_out(reg_name);
	}
	else if(ARMV7L == Architecture)
	{
		emit_out("'9' R0 '0' ");
		emit_out(reg_name);
		emit_out(" MUL R0 ARITH2_ALWAYS");
	}
	else if(AARCH64 == Architecture)
	{
		emit_out("MUL_X0_");
		emit_out(reg_name);
		emit_out("_X0");
	}
	else if(RISCV32 == Architecture || RISCV64 == Architecture)
	{
		emit_out("rd_a0 rs1_");
		emit_out(reg_name);
		emit_out(" rs2_a0 mul");
	}

	if(note == NULL)
	{
		emit_out("\n");
	}
	else
	{
		emit_out(" # ");
		emit_out(note);
		emit_out("\n");
	}
}

void emit_mul_register_zero_with_immediate(int value, char* note)
{
	emit_push(REGISTER_ONE, note);
	emit_load_immediate(REGISTER_ONE, value, note);
	emit_mul_into_register_zero(REGISTER_ONE, note);
	emit_pop(REGISTER_ONE, note);
}

void emit_move(int destination_reg, int source_reg, char* note)
{
	char* destination_name = register_from_string(destination_reg);
	char* source_name = register_from_string(source_reg);

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		emit_out("COPY R");
		emit_out(destination_name);
		emit_out(" R");
		emit_out(source_name);
	}
	else if(X86 == Architecture || AMD64 == Architecture)
	{
		emit_out("mov_");
		emit_out(destination_name);
		emit_out(",");
		emit_out(source_name);
	}
	else if(ARMV7L == Architecture)
	{
		emit_out("'0' ");
		emit_out(source_name);
		emit_out(" ");
		emit_out(destination_name);
		emit_out(" NO_SHIFT MOVE_ALWAYS");
	}
	else if(AARCH64 == Architecture)
	{
		emit_out("SET_");
		emit_out(destination_name);
		emit_out("_FROM_");
		emit_out(source_name);
	}
	else if(RISCV32 == Architecture || RISCV64 == Architecture)
	{
		emit_out("rd_");
		emit_out(destination_name);
		emit_out(" rs1_");
		emit_out(source_name);
		emit_out(" mv");
	}

	if(note == NULL)
	{
		emit_out("\n");
	}
	else
	{
		emit_out(" # ");
		emit_out(note);
		emit_out("\n");
	}
}

void emit_load_relative_to_register(int destination, int offset_register, int value, char* note)
{
	char* destination_name = register_from_string(destination);
	char* offset_name = register_from_string(offset_register);
	char* value_string = int2str(value, 10, TRUE);

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		emit_out("ADDI R");
		emit_out(destination_name);
		emit_out(" R");
		emit_out(offset_name);
		emit_out(" ");
		emit_out(value_string);
	}
	else if((X86 == Architecture) || (AMD64 == Architecture))
	{
		emit_out("lea_");
		emit_out(destination_name);
		emit_out(",[");
		emit_out(offset_name);
		emit_out("+DWORD] %");
		emit_out(value_string);
	}
	else if(ARMV7L == Architecture)
	{
		if((127 >= value) && (value >= -128))
		{
			emit_out("!");
			emit_out(value_string);
			emit_out(" ");
			emit_out(destination_name);
			emit_out(" SUB ");
			emit_out(offset_name);
			emit_out(" ARITH_ALWAYS\n");
		}
		else
		{
			emit_push(REGISTER_ONE, note);
			emit_load_immediate(REGISTER_ONE, value, note);
			emit_out("'0' R0 R0 SUB R1 ARITH2_ALWAYS\n");
			emit_pop(REGISTER_ONE, note);
		}
	}
	else if(AARCH64 == Architecture)
	{
		emit_move(destination, offset_register, note);
		emit_push(REGISTER_ONE, note);
		emit_load_immediate(REGISTER_ONE, value, note);
		emit_out("SUB_");
		emit_out(destination_name);
		emit_out("_");
		emit_out(destination_name);
		emit_out("_X1\n");
		emit_pop(REGISTER_ONE, note);
	}
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
	{
		if((2047 >= value) && (value >= -2048))
		{
			emit_out("rd_");
			emit_out(destination_name);
			emit_out(" rs1_");
			emit_out(offset_name);
			emit_out(" !");
			emit_out(value_string);
			emit_out(" addi");
		}
		else
		{
			emit_out("rd_");
			emit_out(destination_name);
			emit_out(" ~");
			emit_out(value_string);
			emit_out(" lui\n");

			emit_out("rd_");
			emit_out(destination_name);
			emit_out(" rs1_");
			emit_out(offset_name);
			emit_out(" !");
			emit_out(value_string);
			emit_out(" addi");
		}
	}


	if(note == NULL)
	{
		emit_out("\n");
	}
	else
	{
		emit_out(" # ");
		emit_out(note);
		emit_out("\n");
	}
}

void emit_dereference(int reg, char* note)
{
	char* reg_name = register_from_string(reg);
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		emit_out("LOAD R");
		emit_out(reg_name);
		emit_out(" R");
		emit_out(reg_name);
		emit_out(" 0");
	}
	else if(X86 == Architecture || AMD64 == Architecture)
	{
		emit_out("mov_");
		emit_out(reg_name);
		emit_out(",[");
		emit_out(reg_name);
		emit_out("]");
	}
	else if(ARMV7L == Architecture)
	{
		emit_out("!0 ");
		emit_out(reg_name);
		emit_out(" LOAD32 ");
		emit_out(reg_name);
		emit_out(" MEMORY");
	}
	else if(AARCH64 == Architecture)
	{
		emit_out("DEREF_");
		emit_out(reg_name);
	}
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
	{
		emit_out("rd_");
		emit_out(reg_name);
		emit_out(" rs1_");
		emit_out(reg_name);
		if(RISCV32 == Architecture) emit_out(" lw");
		else emit_out(" ld");
	}

	if(note == NULL)
	{
		emit_out("\n");
	}
	else
	{
		emit_out(" # ");
		emit_out(note);
		emit_out("\n");
	}
}

void emit_push(int reg, char* note)
{
	char* reg_name = register_from_string(reg);
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		emit_out("PUSHR R");
		emit_out(reg_name);
		emit_out(" R15");
	}
	else if(X86 == Architecture || AMD64 == Architecture)
	{
		emit_out("push_");
		emit_out(reg_name);
	}
	else if(ARMV7L == Architecture)
	{
		emit_out("{");
		emit_out(reg_name);
		emit_out("} PUSH_ALWAYS");
	}
	else if(AARCH64 == Architecture)
	{
		emit_out("PUSH_");
		emit_out(reg_name);
	}
	else if(RISCV32 == Architecture)
	{
		emit_out("rd_sp rs1_sp !-4 addi\nrs1_sp rs2_");
		emit_out(reg_name);
		emit_out(" sw");
	}
	else if(RISCV64 == Architecture)
	{
		emit_out("rd_sp rs1_sp !-8 addi\nrs1_sp rs2_");
		emit_out(reg_name);
		emit_out(" sd");
	}

	if(note == NULL)
	{
		emit_out("\n");
	}
	else
	{
		emit_out(" # ");
		emit_out(note);
		emit_out("\n");
	}
}

void emit_pop(int reg, char* note)
{
	char* reg_name = register_from_string(reg);
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		emit_out("POPR R");
		emit_out(reg_name);
		emit_out(" R15");
	}
	else if(X86 == Architecture || AMD64 == Architecture)
	{
		emit_out("pop_");
		emit_out(reg_name);
	}
	else if(ARMV7L == Architecture)
	{
		emit_out("{");
		emit_out(reg_name);
		emit_out("} POP_ALWAYS");
	}
	else if(AARCH64 == Architecture)
	{
		emit_out("POP_");
		emit_out(reg_name);
	}
	else if(RISCV32 == Architecture)
	{
		emit_out("rd_");
		emit_out(reg_name);
		emit_out(" rs1_sp lw\nrd_sp rs1_sp !4 addi");
	}
	else if(RISCV64 == Architecture)
	{
		emit_out("rd_");
		emit_out(reg_name);
		emit_out(" rs1_sp ld\nrd_sp rs1_sp !8 addi");
	}

	if(note == NULL)
	{
		emit_out("\n");
	}
	else
	{
		emit_out(" # ");
		emit_out(note);
		emit_out("\n");
	}
}

int type_is_pointer(struct type* type_size)
{
	return type_size->type != type_size || (type_size->options & TO_FUNCTION_POINTER);
}

int type_is_struct_or_union(struct type* type_size)
{
	return type_size->members != NULL;
}

struct token_list* uniqueID(char* s, struct token_list* l, char* num)
{
	l = emit("\n", emit(num, emit("_", emit(s, l))));
	return l;
}

void uniqueID_out(char* s, char* num)
{
	output_list = uniqueID(s, output_list, num);
}

char* create_unique_id(char* prefix, char* s, char* num)
{
	char* buf = calloc(MAX_STRING, sizeof(char));
	int written = copy_string(buf, prefix, MAX_STRING);
	written = copy_string(buf + written, s, MAX_STRING - written) + written;
	written = copy_string(buf + written, "_", MAX_STRING - written) + written;
	copy_string(buf + written, num, MAX_STRING - written);

	return buf;
}

struct token_list* sym_declare(char *s, struct type* t, struct token_list* list)
{
	struct token_list* a = calloc(1, sizeof(struct token_list));
	require(NULL != a, "Exhausted memory while attempting to declare a symbol\n");
	a->next = list;
	a->s = s;
	a->type = t;
	a->array_modifier = 1;
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
	else if(global_token->s[0] == ':')
	{
		/* Switch labels have the : prepended to the front like labels.
		 * This is just a normal integer or constant.*/

		struct token_list* lookup = sym_lookup(global_token->s + 1, global_constant_list);
		if(lookup != NULL)
		{
			return strtoint(lookup->arguments->s);
		}
		else
		{
			return strtoint(global_token->s + 1);
		}
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
	emit_move(REGISTER_TEMP, REGISTER_STACK, "Copy new base pointer");

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

	if(TRUE == is_function_pointer)
	{
		int value = s->depth;

		emit_load_relative_to_register(REGISTER_ZERO, REGISTER_BASE, value, "function pointer call");
		emit_dereference(REGISTER_ZERO, "function pointer call");

		if(ARMV7L == Architecture)
		{
			emit_push(REGISTER_RETURN, "Protect the old link register");
		}

		emit_move(REGISTER_BASE, REGISTER_TEMP, "function pointer call");

		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
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
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
		{
			emit_out("rd_ra rs1_a0 jalr\n");
		}
	}
	else
	{
		if(ARMV7L == Architecture)
		{
			emit_push(REGISTER_RETURN, "Protect the old link register");
		}

		emit_move(REGISTER_BASE, REGISTER_TEMP, "function call");

		if((KNIGHT_NATIVE == Architecture) || (KNIGHT_POSIX == Architecture))
		{
			emit_load_named_immediate(REGISTER_ZERO, "FUNCTION_", s->s, "function call");
			emit_out("CALL R0 R15\n");
		}
		else if((X86 == Architecture) || (AMD64 == Architecture))
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
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
		{
			emit_out("rd_ra $FUNCTION_");
			emit_out(s->s);
			emit_out(" jal\n");
		}
	}

	for(; passed > 0; passed = passed - 1)
	{
		emit_pop(REGISTER_ONE, "_process_expression_locals");
	}

	emit_pop(REGISTER_BASE, "Restore old base pointer");
	if((AARCH64 == Architecture) || (RISCV64 == Architecture) || (RISCV32 == Architecture))
	{
		emit_pop(REGISTER_RETURN, "Restore old return pointer (link)");
	}
	emit_pop(REGISTER_TEMP, "Restore temp register");
}

void constant_load(char* s)
{
	emit_load_immediate(REGISTER_ZERO, strtoint(s), "constant load");
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

int num_dereference_after_postfix;
void postfix_expr_stub(void);
void variable_load(struct token_list* a, int num_dereference)
{
	require(NULL != global_token, "incomplete variable load received\n");
	if((a->type->options & TO_FUNCTION_POINTER) && match("(", global_token->s))
	{
		function_call(a, TRUE);
		return;
	}
	current_target = a->type;

	emit_load_relative_to_register(REGISTER_ZERO, REGISTER_BASE, a->depth, "variable load");

	if(TRUE == Address_of) return;
	if(match(".", global_token->s))
	{
		postfix_expr_stub();
		return;
	}

	int is_local_array = match("[", global_token->s) && (a->options & TLO_LOCAL_ARRAY);
	int is_prefix_operator = match("++", global_token->prev->prev->s) || match("--", global_token->prev->prev->s);
	int is_postfix_operator = match("++", global_token->s) || match("--", global_token->s);
	if(!match("=", global_token->s) && !is_compound_assignment(global_token->s) && !is_local_array && !is_prefix_operator && !is_postfix_operator)
	{
		emit_out(load_value(current_target->size, current_target->is_signed));
		while (num_dereference > 0)
		{
			current_target = current_target->type;
			emit_out(load_value(current_target->size, current_target->is_signed));
			num_dereference = num_dereference - 1;
		}
		return;
	}

	num_dereference_after_postfix = num_dereference;
	if(!is_postfix_operator)
	{
		while (num_dereference > 0)
		{
			emit_out(load_value(current_target->size, current_target->is_signed));
			current_target = current_target->type;
			num_dereference = num_dereference - 1;
		}
	}
}

void function_load(struct token_list* a)
{
	require(NULL != global_token, "incomplete function load\n");
	if(match("(", global_token->s))
	{
		function_call(a, FALSE);
		return;
	}

	emit_load_named_immediate(REGISTER_ZERO, "FUNCTION_", a->s, "function load");
}

void global_load(struct token_list* a)
{
	current_target = a->type;
	emit_load_named_immediate(REGISTER_ZERO, "GLOBAL_", a->s, "global load");

	require(NULL != global_token, "unterminated global load\n");
	if(TRUE == Address_of) return;
	if(match(".", global_token->s))
	{
		postfix_expr_stub();
		return;
	}
	if(match("=", global_token->s) || is_compound_assignment(global_token->s)) return;

	emit_out(load_value(register_size, current_target->is_signed));
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

void primary_expr_variable(void)
{
	int num_dereference = 0;
	while(global_token->s[0] == '*') {
		require_extra_token();
		num_dereference = num_dereference + 1;
	}
	char* s = global_token->s;
	require_extra_token();
	struct token_list* a = sym_lookup(s, global_constant_list);
	if(NULL != a)
	{
		constant_load(a->arguments->s);
		return;
	}

	a = static_variable_lookup(s);
	if(NULL != a)
	{
		global_load(a);
		return;
	}

	a = sym_lookup(s, function->locals);
	if(NULL != a)
	{
		variable_load(a, num_dereference);
		return;
	}

	a = sym_lookup(s, function->arguments);
	if(NULL != a)
	{
		variable_load(a, num_dereference);
		return;
	}

	a = sym_lookup(s, global_function_list);
	if(NULL != a)
	{
		function_load(a);
		return;
	}

	a = sym_lookup(s, global_symbol_list);
	if(NULL != a)
	{
		global_load(a);
		return;
	}

	line_error();
	fputs(s ,stderr);
	fputs(" is not a defined symbol\n", stderr);
	exit(EXIT_FAILURE);
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

	emit_push(REGISTER_ONE, "pointer arithmetic");

	emit_mul_register_zero_with_immediate(current_target->type->size, "pointer arithmetic");

	emit_pop(REGISTER_ONE, "pointer arithmetic");
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

	/* We don't yet support assigning structs to structs */
	if((!match("=", global_token->s) && !is_compound_assignment(global_token->s) && (register_size >= i->size)))
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
	if(match("=", global_token->s) || is_compound_assignment(global_token->s)) return;
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

	emit_add(REGISTER_ZERO, REGISTER_ONE, "primary expr array");

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
struct type* type_name(void);
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
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		arithmetic_recursion(additive_expr_a, "ADD R0 R1 R0\n", "ADDU R0 R1 R0\n", "+", additive_expr_stub_b);
		arithmetic_recursion(additive_expr_a, "SUB R0 R1 R0\n", "SUBU R0 R1 R0\n", "-", additive_expr_stub_b);
	}
	else if(X86 == Architecture)
	{
		arithmetic_recursion(additive_expr_a, "add_eax,ebx\n", "add_eax,ebx\n", "+", additive_expr_stub_b);
		arithmetic_recursion(additive_expr_a, "sub_ebx,eax\nmov_eax,ebx\n", "sub_ebx,eax\nmov_eax,ebx\n", "-", additive_expr_stub_b);
	}
	else if(AMD64 == Architecture)
	{
		arithmetic_recursion(additive_expr_a, "add_rax,rbx\n", "add_rax,rbx\n", "+", additive_expr_stub_b);
		arithmetic_recursion(additive_expr_a, "sub_rbx,rax\nmov_rax,rbx\n", "sub_rbx,rax\nmov_rax,rbx\n", "-", additive_expr_stub_b);
	}
	else if(ARMV7L == Architecture)
	{
		arithmetic_recursion(additive_expr_a, "'0' R0 R0 ADD R1 ARITH2_ALWAYS\n", "'0' R0 R0 ADD R1 ARITH2_ALWAYS\n", "+", additive_expr_stub_b);
		arithmetic_recursion(additive_expr_a, "'0' R0 R0 SUB R1 ARITH2_ALWAYS\n", "'0' R0 R0 SUB R1 ARITH2_ALWAYS\n", "-", additive_expr_stub_b);
	}
	else if(AARCH64 == Architecture)
	{
		general_recursion(additive_expr_a, "ADD_X0_X1_X0\n", "+", additive_expr_stub_b);
		general_recursion(additive_expr_a, "SUB_X0_X1_X0\n", "-", additive_expr_stub_b);
	}
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
	{
		general_recursion(additive_expr_a, "rd_a0 rs1_a1 rs2_a0 add\n", "+", additive_expr_stub_b);
		general_recursion(additive_expr_a, "rd_a0 rs1_a1 rs2_a0 sub\n", "-", additive_expr_stub_b);
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
		arithmetic_recursion(additive_expr_c, "cmp\nsetl_al\nmovzx_eax,al\n", "cmp\nsetb_al\nmovzx_eax,al\n", "<", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "cmp\nsetle_al\nmovzx_eax,al\n", "cmp\nsetbe_al\nmovzx_eax,al\n", "<=", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "cmp\nsetge_al\nmovzx_eax,al\n", "cmp\nsetae_al\nmovzx_eax,al\n", ">=", relational_expr_stub);
		arithmetic_recursion(additive_expr_c, "cmp\nsetg_al\nmovzx_eax,al\n", "cmp\nseta_al\nmovzx_eax,al\n", ">", relational_expr_stub);
		general_recursion(additive_expr_c, "cmp\nsete_al\nmovzx_eax,al\n", "==", relational_expr_stub);
		general_recursion(additive_expr_c, "cmp\nsetne_al\nmovzx_eax,al\n", "!=", relational_expr_stub);
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

	if(match("sizeof", global_token->s)) constant_load(int2str(unary_expr_sizeof(), 10, TRUE));
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
		else if(X86 == Architecture) emit_out("cmp\nseta_al\nmovzx_eax,al\n");
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
		expression();
		require_match("Error in Primary expression\nDidn't get )\n", ")");
	}
	else if(global_token->s[0] == '\'') primary_expr_char();
	else if(global_token->s[0] == '"') primary_expr_string();
	else if(in_set(global_token->s[0], "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_")) primary_expr_variable();
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
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
		{
			if(is_signed) operation = "ADD R0 R1 R0\n";
			else operation = "ADDU R0 R1 R0\n";
		}
		else emit_add(REGISTER_ZERO, REGISTER_ONE, "compound operation");
	}
	else if(match("-=", operator))
	{
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
		{
			if(is_signed) operation = "SUB R0 R1 R0\n";
			else operation =  "SUBU R0 R1 R0\n";
		}
		else if(X86 == Architecture) operation = "sub_ebx,eax\nmov_eax,ebx\n";
		else if(AMD64 == Architecture) operation = "sub_rbx,rax\nmov_rax,rbx\n";
		else if(ARMV7L == Architecture) operation = "'0' R0 R0 SUB R1 ARITH2_ALWAYS\n";
		else if(AARCH64 == Architecture) operation = "SUB_X0_X1_X0\n";
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) operation = "rd_a0 rs1_a1 rs2_a0 sub\n";
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
	if(match("register", s)) return TRUE;
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

void process_static_variable(int);
/* Process local variable */
void collect_local(void)
{
	if(NULL != break_target_func)
	{
		process_static_variable(TRUE);
		return;
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
	unsigned i;
	char* name;

	do
	{
		if(global_token->s[0] == '(')
		{
			require_extra_token();

			require_match("Required '*' after '(' in local function pointer.\n", "*");
			require(NULL != global_token->s, "NULL token in local function pointer");

			name = global_token->s;
			require_extra_token();

			require_match("Required ')' after name in local function pointer.\n", ")");
			require_match("Required '(' after ')' in local function pointer.\n", "(");

			while(global_token->s[0] != ')')
			{
				type_name();

				if(global_token->s[0] == ',')
				{
					require_extra_token();
				}
			}
			require_extra_token();

			current_type = function_pointer;
		}
		else
		{
			name = global_token->s;
			require_extra_token();
		}

		a = sym_declare(name, current_type, list_to_append_to);
		list_to_append_to = a;

		if(match("main", function->s) && (NULL == function->locals))
		{
			if(KNIGHT_NATIVE == Architecture) a->depth = register_size;
			else if(KNIGHT_POSIX == Architecture) a->depth = 20;
			else if(X86 == Architecture) a->depth = -20;
			else if(AMD64 == Architecture) a->depth = -40;
			else if(ARMV7L == Architecture) a->depth = 16;
			else if(AARCH64 == Architecture) a->depth = 32; /* argc, argv, envp and the local (8 bytes each) */
			else if(RISCV32 == Architecture) a->depth = -16;
			else if(RISCV64 == Architecture) a->depth = -32;
		}
		else if((NULL == function->arguments) && (NULL == function->locals))
		{
			if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) a->depth = register_size;
			else if(X86 == Architecture) a->depth = -8;
			else if(AMD64 == Architecture) a->depth = -16;
			else if(ARMV7L == Architecture) a->depth = 8;
			else if(AARCH64 == Architecture) a->depth = register_size;
			else if(RISCV32 == Architecture) a->depth = -4;
			else if(RISCV64 == Architecture) a->depth = -8;
		}
		else if(NULL == function->locals)
		{
			if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) a->depth = function->arguments->depth + 8;
			else if(X86 == Architecture) a->depth = function->arguments->depth - 8;
			else if(AMD64 == Architecture) a->depth = function->arguments->depth - 16;
			else if(ARMV7L == Architecture) a->depth = function->arguments->depth + 8;
			else if(AARCH64 == Architecture) a->depth = function->arguments->depth + register_size;
			else if(RISCV32 == Architecture) a->depth = function->arguments->depth - 4;
			else if(RISCV64 == Architecture) a->depth = function->arguments->depth - 8;
		}
		else
		{
			if(stack_direction == STACK_DIRECTION_PLUS)
			{
				a->depth = function->locals->depth + register_size;
			}
			else
			{
				a->depth = function->locals->depth - register_size;
			}
		}

		function->locals = a;

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

		/* Adjust the depth of local structs. When stack grows downwards, we want them to
		   start at the bottom of allocated space. */
		struct_depth_adjustment = (ceil_div(a->type->size * a->array_modifier, register_size) - 1) * register_size;
		if(stack_direction == STACK_DIRECTION_PLUS)
		{
			a->depth = a->depth + struct_depth_adjustment;
		}
		else
		{
			a->depth = a->depth - struct_depth_adjustment;
		}

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
		}

		i = ceil_div(a->type->size * a->array_modifier, register_size);
		while(i != 0)
		{
			emit_push(REGISTER_ZERO, a->s);
			i = i - 1;
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

	emit_out("# IF_");
	uniqueID_out(function->s, number_string);

	global_token = global_token->next;
	require_match("ERROR in process_if\nMISSING (\n", "(");
	expression();

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("JUMP.Z R0 @ELSE_");
	else if(X86 == Architecture) emit_out("test_eax,eax\nje %ELSE_");
	else if(AMD64 == Architecture) emit_out("test_rax,rax\nje %ELSE_");
	else if(ARMV7L == Architecture) emit_out("!0 CMPI8 R0 IMM_ALWAYS\n^~ELSE_");
	else if(AARCH64 == Architecture) emit_out("CBNZ_X0_PAST_BR\nLOAD_W16_AHEAD\nSKIP_32_DATA\n&ELSE_");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("rs1_a0 @8 bnez\n$ELSE_");

	uniqueID_out(function->s, number_string);
	if(ARMV7L == Architecture) emit_out(" JUMP_EQUAL\n");
	else if(AARCH64 == Architecture) emit_out("\nBR_X16\n");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("jal\n");

	require_match("ERROR in process_if\nMISSING )\n", ")");
	statement();
	require(NULL != global_token, "Reached EOF inside of function\n");

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("JUMP @_END_IF_");
	else if(X86 == Architecture) emit_out("jmp %_END_IF_");
	else if(AMD64 == Architecture) emit_out("jmp %_END_IF_");
	else if(ARMV7L == Architecture) emit_out("^~_END_IF_");
	else if(AARCH64 == Architecture) emit_out("LOAD_W16_AHEAD\nSKIP_32_DATA\n&_END_IF_");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("$_END_IF_");

	uniqueID_out(function->s, number_string);
	if(ARMV7L == Architecture) emit_out(" JUMP_ALWAYS\n");
	else if(AARCH64 == Architecture) emit_out("\nBR_X16\n");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("jal\n");

	emit_out(":ELSE_");
	uniqueID_out(function->s, number_string);

	if(match("else", global_token->s))
	{
		require_extra_token();
		statement();
		require_token();
	}
	emit_out(":_END_IF_");
	uniqueID_out(function->s, number_string);
}

void process_case(void)
{
process_case_iter:
	require(NULL != global_token, "incomplete case statement\n");
	if(match("case", global_token->s)) return;
	if(match(":default", global_token->s)) return;

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

	emit_out("# switch_");
	uniqueID_out(function->s, number_string);

	/* get what we are casing on */
	global_token = global_token->next;
	require_match("ERROR in process_switch\nMISSING (\n", "(");
	expression();
	require_match("ERROR in process_switch\nMISSING )\n", ")");

	/* Put the value in R1 as it is currently in R0 */
	emit_move(REGISTER_ONE, REGISTER_ZERO, "process switch");

	/* Jump to the switch table */
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("JUMP @_SWITCH_TABLE_");
	else if(X86 == Architecture) emit_out("jmp %_SWITCH_TABLE_");
	else if(AMD64 == Architecture) emit_out("jmp %_SWITCH_TABLE_");
	else if(ARMV7L == Architecture) emit_out("^~_SWITCH_TABLE_");
	else if(AARCH64 == Architecture) emit_out("LOAD_W16_AHEAD\nSKIP_32_DATA\n&_SWITCH_TABLE_");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("$_SWITCH_TABLE_");

	uniqueID_out(function->s, number_string);
	if(ARMV7L == Architecture) emit_out(" JUMP_ALWAYS\n");
	else if(AARCH64 == Architecture) emit_out("\nBR_X16\n");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("jal\n");

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
			uniqueID_out(function->s, number_string);
			require_extra_token();
			process_case();
			require_token();
		}
		else line_error();
		goto process_switch_iter;
	}
	else if(match(":default", global_token->s))
	{ /* because of how M2-Planet treats labels */
		require_extra_token();
		emit_out(":_SWITCH_DEFAULT_");
		uniqueID_out(function->s, number_string);

		require(NULL != global_token, "recieved EOF before switch closing }\n");
		/* collect statements until } */
		while(!match("}", global_token->s))
		{
			statement();
			require(NULL != global_token, "recieved EOF before switch closing }\n");
		}

		/* jump over the switch table */
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("JUMP @_SWITCH_END_");
		else if(X86 == Architecture) emit_out("jmp %_SWITCH_END_");
		else if(AMD64 == Architecture) emit_out("jmp %_SWITCH_END_");
		else if(ARMV7L == Architecture) emit_out("^~_SWITCH_END_");
		else if(AARCH64 == Architecture) emit_out("LOAD_W16_AHEAD\nSKIP_32_DATA\n&_SWITCH_END_");
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("$_SWITCH_END_");

		uniqueID_out(function->s, number_string);
		if(ARMV7L == Architecture) emit_out(" JUMP_ALWAYS\n");
		else if(AARCH64 == Architecture) emit_out("\nBR_X16\n");
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("jal\n");
	}

	/* Switch statements must end with } */
	require_match("ERROR in process_switch\nMISSING }\n", "}");

	/* create the table */
	emit_out(":_SWITCH_TABLE_");
	uniqueID_out(function->s, number_string);

	struct case_list* hold;
	while(NULL != backtrack)
	{
		/* put case value in R0 as the switch (value) is in R1 */
		primary_expr_number(backtrack->value);
		hold = backtrack->next;

		/* compare R0 and R1 and jump to case if equal */
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("CMPU R0 R0 R1\nJUMP.E R0 @_SWITCH_CASE_");
		else if(X86 == Architecture) emit_out("cmp\nje %_SWITCH_CASE_");
		else if(AMD64 == Architecture) emit_out("cmp_rbx,rax\nje %_SWITCH_CASE_");
		else if(ARMV7L == Architecture) emit_out("'0' R0 CMP R1 AUX_ALWAYS\n^~_SWITCH_CASE_");
		else if(AARCH64 == Architecture) emit_out("CMP_X1_X0\nSKIP_32_DATA\n&_SWITCH_CASE_");
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("rd_a0 rs1_a0 rs2_a1 sub\nrs1_a0 @8 bnez\n$_SWITCH_CASE_");

		emit_out(backtrack->value);
		emit_out("_");
		uniqueID_out(function->s, number_string);
		if(ARMV7L == Architecture) emit_out(" JUMP_EQUAL\n");
		else if(AARCH64 == Architecture) emit_out("\nSKIP_INST_NE\nBR_X16\n");
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("jal\n");

		free(backtrack);
		backtrack = hold;
	}

	/* Default to :default */
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("JUMP @_SWITCH_DEFAULT_");
	else if(X86 == Architecture) emit_out("jmp %_SWITCH_DEFAULT_");
	else if(AMD64 == Architecture) emit_out("jmp %_SWITCH_DEFAULT_");
	else if(ARMV7L == Architecture) emit_out("^~_SWITCH_DEFAULT_");
	else if(AARCH64 == Architecture) emit_out("SKIP_32_DATA\n&_SWITCH_DEFAULT_");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("$_SWITCH_DEFAULT_");

	uniqueID_out(function->s, number_string);
	if(ARMV7L == Architecture) emit_out(" JUMP_ALWAYS\n");
	else if(AARCH64 == Architecture) emit_out("\nBR_X16\n");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("jal\n");

	/* put the exit of the switch */
	emit_out(":_SWITCH_END_");
	uniqueID_out(function->s, number_string);

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

	emit_out("# FOR_initialization_");
	uniqueID_out(function->s, number_string);

	require_extra_token();

	require_match("ERROR in process_for\nMISSING (\n", "(");
	if(!match(";",global_token->s))
	{
		expression();
	}

	emit_out(":FOR_");
	uniqueID_out(function->s, number_string);

	require_match("ERROR in process_for\nMISSING ;1\n", ";");
	expression();

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("JUMP.Z R0 @FOR_END_");
	else if(X86 == Architecture) emit_out("test_eax,eax\nje %FOR_END_");
	else if(AMD64 == Architecture) emit_out("test_rax,rax\nje %FOR_END_");
	else if(ARMV7L == Architecture) emit_out("!0 CMPI8 R0 IMM_ALWAYS\n^~FOR_END_");
	else if(AARCH64 == Architecture) emit_out("CBNZ_X0_PAST_BR\nLOAD_W16_AHEAD\nSKIP_32_DATA\n&FOR_END_");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("rs1_a0 @8 bnez\n$FOR_END_");
	uniqueID_out(function->s, number_string);
	if(ARMV7L == Architecture) emit_out(" JUMP_EQUAL\n");
	else if(AARCH64 == Architecture) emit_out("\nBR_X16\n");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("jal\n");

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("JUMP @FOR_THEN_");
	else if(X86 == Architecture) emit_out("jmp %FOR_THEN_");
	else if(AMD64 == Architecture) emit_out("jmp %FOR_THEN_");
	else if(ARMV7L == Architecture) emit_out("^~FOR_THEN_");
	else if(AARCH64 == Architecture) emit_out("LOAD_W16_AHEAD\nSKIP_32_DATA\n&FOR_THEN_");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("$FOR_THEN_");
	uniqueID_out(function->s, number_string);
	if(ARMV7L == Architecture) emit_out(" JUMP_ALWAYS\n");
	else if(AARCH64 == Architecture) emit_out("\nBR_X16\n");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("jal\n");

	emit_out(":FOR_ITER_");
	uniqueID_out(function->s, number_string);

	require_match("ERROR in process_for\nMISSING ;2\n", ";");
	expression();

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("JUMP @FOR_");
	else if(X86 == Architecture) emit_out("jmp %FOR_");
	else if(AMD64 == Architecture) emit_out("jmp %FOR_");
	else if(ARMV7L == Architecture) emit_out("^~FOR_");
	else if(AARCH64 == Architecture) emit_out("LOAD_W16_AHEAD\nSKIP_32_DATA\n&FOR_");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("$FOR_");
	uniqueID_out(function->s, number_string);
	if(ARMV7L == Architecture) emit_out(" JUMP_ALWAYS\n");
	else if(AARCH64 == Architecture) emit_out("\nBR_X16\n");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("jal\n");

	emit_out(":FOR_THEN_");
	uniqueID_out(function->s, number_string);

	require_match("ERROR in process_for\nMISSING )\n", ")");
	statement();
	require(NULL != global_token, "Reached EOF inside of function\n");

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("JUMP @FOR_ITER_");
	else if(X86 == Architecture) emit_out("jmp %FOR_ITER_");
	else if(AMD64 == Architecture) emit_out("jmp %FOR_ITER_");
	else if(ARMV7L == Architecture) emit_out("^~FOR_ITER_");
	else if(AARCH64 == Architecture) emit_out("LOAD_W16_AHEAD\nSKIP_32_DATA\n&FOR_ITER_");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("$FOR_ITER_");
	uniqueID_out(function->s, number_string);
	if(ARMV7L == Architecture) emit_out(" JUMP_ALWAYS\n");
	else if(AARCH64 == Architecture) emit_out("\nBR_X16\n");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("jal\n");

	emit_out(":FOR_END_");
	uniqueID_out(function->s, number_string);

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

	break_target_head = "DO_END_";
	continue_target_head = "DO_TEST_";
	break_target_num = number_string;
	break_frame = function->locals;
	break_target_func = function->s;

	emit_out(":DO_");
	uniqueID_out(function->s, number_string);

	require_extra_token();
	statement();
	require_token();

	emit_out(":DO_TEST_");
	uniqueID_out(function->s, number_string);

	require_match("ERROR in process_do\nMISSING while\n", "while");
	require_match("ERROR in process_do\nMISSING (\n", "(");
	expression();
	require_match("ERROR in process_do\nMISSING )\n", ")");
	require_match("ERROR in process_do\nMISSING ;\n", ";");

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("JUMP.NZ R0 @DO_");
	else if(X86 == Architecture) emit_out("test_eax,eax\njne %DO_");
	else if(AMD64 == Architecture) emit_out("test_rax,rax\njne %DO_");
	else if(ARMV7L == Architecture) emit_out("!0 CMPI8 R0 IMM_ALWAYS\n^~DO_");
	else if(AARCH64 == Architecture) emit_out("CBZ_X0_PAST_BR\nLOAD_W16_AHEAD\nSKIP_32_DATA\n&DO_");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("rs1_a0 @DO_END_");
	uniqueID_out(function->s, number_string);
	if(ARMV7L == Architecture) emit_out(" JUMP_NE\n");
	else if(AARCH64 == Architecture) emit_out("\nBR_X16\n");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
	{
		emit_out("beqz\n$DO_");
		uniqueID_out(function->s, number_string);
		emit_out("jal\n");
	}

	emit_out(":DO_END_");
	uniqueID_out(function->s, number_string);

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

	break_target_head = "END_WHILE_";
	continue_target_head = "WHILE_";
	break_target_num = number_string;
	break_frame = function->locals;
	break_target_func = function->s;

	emit_out(":WHILE_");
	uniqueID_out(function->s, number_string);

	global_token = global_token->next;
	require_match("ERROR in process_while\nMISSING (\n", "(");
	expression();

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("JUMP.Z R0 @END_WHILE_");
	else if(X86 == Architecture) emit_out("test_eax,eax\nje %END_WHILE_");
	else if(AMD64 == Architecture) emit_out("test_rax,rax\nje %END_WHILE_");
	else if(ARMV7L == Architecture) emit_out("!0 CMPI8 R0 IMM_ALWAYS\n^~END_WHILE_");
	else if(AARCH64 == Architecture) emit_out("CBNZ_X0_PAST_BR\nLOAD_W16_AHEAD\nSKIP_32_DATA\n&END_WHILE_");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("rs1_a0 @8 bnez\n$END_WHILE_");
	uniqueID_out(function->s, number_string);
	if(ARMV7L == Architecture) emit_out(" JUMP_EQUAL\t");
	else if(AARCH64 == Architecture) emit_out("\nBR_X16\n");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("jal\n");
	emit_out("# THEN_while_");
	uniqueID_out(function->s, number_string);

	require_match("ERROR in process_while\nMISSING )\n", ")");
	statement();
	require(NULL != global_token, "Reached EOF inside of function\n");

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("JUMP @WHILE_");
	else if(X86 == Architecture) emit_out("jmp %WHILE_");
	else if(AMD64 == Architecture) emit_out("jmp %WHILE_");
	else if(ARMV7L == Architecture) emit_out("^~WHILE_");
	else if(AARCH64 == Architecture) emit_out("LOAD_W16_AHEAD\nSKIP_32_DATA\n&WHILE_");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("$WHILE_");
	uniqueID_out(function->s, number_string);
	if(ARMV7L == Architecture) emit_out(" JUMP_ALWAYS\n");
	else if(AARCH64 == Architecture) emit_out("\nBR_X16\n");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("jal\n");
	emit_out(":END_WHILE_");
	uniqueID_out(function->s, number_string);

	break_target_head = nested_break_head;
	break_target_func = nested_break_func;
	break_target_num = nested_break_num;
	continue_target_head = nested_continue_head;
	break_frame = nested_locals;
}

/* Ensure that functions return */
void return_result(void)
{
	require_extra_token();
	if(global_token->s[0] != ';') expression();

	require_match("ERROR in return_result\nMISSING ;\n", ";");

	struct token_list* i;
	unsigned size_local_var;
	for(i = function->locals; NULL != i; i = i->next)
	{
		size_local_var = ceil_div(i->type->size * i->array_modifier, register_size);
		while(size_local_var != 0)
		{
			emit_pop(REGISTER_ONE, "_return_result_locals");
			size_local_var = size_local_var - 1;
		}
	}

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("RET R15\n");
	else if(X86 == Architecture) emit_out("ret\n");
	else if(AMD64 == Architecture) emit_out("ret\n");
	else if(ARMV7L == Architecture) emit_out("'1' LR RETURN\n");
	else if(AARCH64 == Architecture) emit_out("RETURN\n");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("ret\n");
}

void process_break(void)
{
	if(NULL == break_target_head)
	{
		line_error();
		fputs("Not inside of a loop or case statement\n", stderr);
		exit(EXIT_FAILURE);
	}
	struct token_list* i = function->locals;
	while(i != break_frame)
	{
		if(NULL == i) break;
		emit_pop(REGISTER_ONE, "break_cleanup_locals");
		i = i->next;
	}
	require_extra_token();

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("JUMP @");
	else if(X86 == Architecture) emit_out("jmp %");
	else if(AMD64 == Architecture) emit_out("jmp %");
	else if(ARMV7L == Architecture) emit_out("^~");
	else if(AARCH64 == Architecture) emit_out("LOAD_W16_AHEAD\nSKIP_32_DATA\n&");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("$");

	emit_out(break_target_head);
	emit_out(break_target_func);
	emit_out("_");
	emit_out(break_target_num);
	if(ARMV7L == Architecture) emit_out(" JUMP_ALWAYS");
	else if(AARCH64 == Architecture) emit_out("\nBR_X16");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out(" jal");
	emit_out("\n");
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

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("JUMP @");
	else if(X86 == Architecture) emit_out("jmp %");
	else if(AMD64 == Architecture) emit_out("jmp %");
	else if(ARMV7L == Architecture) emit_out("^~");
	else if(AARCH64 == Architecture) emit_out("LOAD_W16_AHEAD\nSKIP_32_DATA\n&");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("$");

	emit_out(continue_target_head);
	emit_out(break_target_func);
	emit_out("_");
	emit_out(break_target_num);
	if(ARMV7L == Architecture) emit_out(" JUMP_ALWAYS");
	else if(AARCH64 == Architecture) emit_out("\nBR_X16");
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out(" jal");
	emit_out("\n");
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

	/* Clean up any locals added */

	if(((X86 == Architecture) && !match("ret\n", output_list->s)) ||
	   ((AMD64 == Architecture) && !match("ret\n", output_list->s)) ||
	   (((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) && !match("RET R15\n", output_list->s)) ||
	   ((ARMV7L == Architecture) && !match("'1' LR RETURN\n", output_list->s)) ||
	   ((AARCH64 == Architecture) && !match("RETURN\n", output_list->s)) ||
	   (((RISCV32 == Architecture) || (RISCV64 == Architecture)) && !match("ret\n", output_list->s)))
	{
		struct token_list* i;
		int j;
		for(i = function->locals; frame != i; i = i->next)
		{
			j = ceil_div(i->type->size * i->array_modifier, register_size);
			while (j != 0) {
				emit_pop(REGISTER_ONE, "_recursive_statement_locals");
				j = j - 1;
			}
		}
	}
	function->locals = frame;
}

/* Variables inside loops are currently just global variables */
void process_static_variable(int is_loop_variable)
{
	maybe_bootstrap_error("static local variable");

	struct type* type_size = type_name();

	struct static_variable_list* variable = calloc(1, sizeof(struct static_variable_list));
	variable->next = function_static_variables_list;
	function_static_variables_list = variable;
	variable->local_variable_name = global_token->s;
	char* new_name = calloc(MAX_STRING, sizeof(char));

	int offset = copy_string(new_name, function->s, MAX_STRING);
	offset = offset + copy_string(new_name + offset, "_", MAX_STRING - offset);
	copy_string(new_name + offset, variable->local_variable_name, MAX_STRING - offset);

	variable->global_variable = sym_declare(new_name, type_size, NULL);
	require_extra_token();

	if(match(";", global_token->s))
	{
		global_variable_definition(type_size, new_name);
		return;
	}

	/* Deal with assignment to a global variable */
	if(match("=", global_token->s))
	{
		if(is_loop_variable)
		{
			global_variable_definition(type_size, new_name);
			require(NULL != global_token, "NULL token received in loop variable assignment");

			/* global_load requires the global_token to see the current token as =
			 * in order to prevent loading the value rather than the address. */
			global_token = global_token->prev;
			global_load(variable->global_variable);
			require_extra_token();

			emit_push(REGISTER_ZERO, "_process_expression1");

			expression();

			emit_pop(REGISTER_ONE, "static_loop_variable");

			emit_out(store_value(type_size->size));
			require_match("Missing ; from loop variable.\n", ";");
		}
		else
		{
			global_assignment(new_name, type_size);
		}
		return;
	}

	/* Deal with global static arrays */
	if(match("[", global_token->s))
	{
		variable->global_variable->array_modifier = global_static_array(type_size, new_name);
	}
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

	if(global_token->s[0] == '{')
	{
		recursive_statement();
	}
	else if(':' == global_token->s[0])
	{
		emit_out(global_token->s);
		emit_out("\t#C goto label\n");
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
		if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) emit_out("JUMP @");
		else if(X86 == Architecture) emit_out("jmp %");
		else if(AMD64 == Architecture) emit_out("jmp %");
		else if(ARMV7L == Architecture) emit_out("^~");
		else if(AARCH64 == Architecture) emit_out("LOAD_W16_AHEAD\nSKIP_32_DATA\n&");
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out("$");
		emit_out(global_token->s);
		if(ARMV7L == Architecture) emit_out(" JUMP_ALWAYS");
		else if(AARCH64 == Architecture) emit_out("\nBR_X16");
		else if((RISCV32 == Architecture) || (RISCV64 == Architecture)) emit_out(" jal");
		emit_out("\n");
		global_token = global_token->next;
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

		process_static_variable(FALSE);
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
			require(global_token->next != NULL, "EOF in variadic parameter");
			require(global_token->next->s[0] == '.', "Invalid token '.' in macro parameter list");
			require(global_token->next->next != NULL, "EOF in second variadic parameter");
			require(global_token->next->next->s[0] == '.', "Invalid tokens '..' in macro parameter list");

			maybe_bootstrap_error("variadic functions");

			line_error();
			fputs("Variadic functions are not supported.\n", stderr);
			exit(EXIT_FAILURE);
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
			a = sym_declare(global_token->s, type_size, function->arguments);
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
	function = sym_declare(global_token->prev->s, NULL, global_function_list);

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
		emit_out(":FUNCTION_");
		emit_out(function->s);
		emit_out("\n");
		/* If we add any statics we don't want them globally available */
		function_static_variables_list = NULL;
		statement();
		/* Just to be sure this doesn't escape the function somehow. */
		function_static_variables_list = NULL;

		/* C99 5.1.2.2.3 Program termination
		 * [..] reaching the } that terminates the main function returns a value of 0.
		 * */
		int is_main = match(function->s, "main");

		/* Prevent duplicate RETURNS */
		if(((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture)) && !match("RET R15\n", output_list->s))
		{
			if(is_main) emit_load_immediate(REGISTER_ZERO, 0, "declare function");
			emit_out("RET R15\n");
		}
		else if((X86 == Architecture || AMD64 == Architecture || RISCV32 == Architecture || RISCV64 == Architecture)
				&& !match("ret\n", output_list->s))
		{
			if(is_main) emit_load_immediate(REGISTER_ZERO, 0, "declare function");
			emit_out("ret\n");
		}
		else if((ARMV7L == Architecture) && !match("'1' LR RETURN\n", output_list->s))
		{
			if(is_main) emit_load_immediate(REGISTER_ZERO, 0, "declare function");
			emit_out("'1' LR RETURN\n");
		}
		else if((AARCH64 == Architecture) && !match("RETURN\n", output_list->s))
		{
			if(is_main) emit_load_immediate(REGISTER_ZERO, 0, "declare function");
			emit_out("RETURN\n");
		}
	}
}

void global_constant(void)
{
	require_extra_token();
	global_constant_list = sym_declare(global_token->s, NULL, global_constant_list);

	require(NULL != global_token->next, "CONSTANT lacks a value\n");
	if(match("sizeof", global_token->next->s))
	{
		global_token = global_token->next->next;
		require_match("ERROR in CONSTANT with sizeof\nMissing (\n", "(");
		struct type* a = type_name();
		require_match("ERROR in CONSTANT with sizeof\nMissing )\n", ")");
		global_token->prev->s = int2str(a->size, 10, TRUE);
		global_constant_list->arguments = global_token->prev;
	}
	else
	{
		global_constant_list->arguments = global_token->next;
		require_extra_token();
		require_extra_token();
	}
}

struct type* typedef_function_pointer(void)
{
	require_extra_token(); /* skip '(' */
	require_match("Invalid token in function pointer parsing, expected '*'.\n", "*");
	require(NULL != global_token, "Received EOF while reading typedef function pointer\n");

	char* name = global_token->s;

	require_extra_token();
	require_match("Invalid token in function pointer parsing, expected ')'.\n", ")");
	require_match("Invalid token in function pointer parsing, expected '('.\n", "(");

	while(global_token->s[0] != ')')
	{
		type_name();

		if(global_token->s[0] == ',')
		{
			require_extra_token();
		}
	}
	require_extra_token(); /* skip ')' */

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
		char* hex_table = "0123456789ABCDEF";
		char* string;

		string = calloc(6, sizeof(char));
		string[0] = '\'';
		string[1] = hex_table[value >> 4];
		string[2] = hex_table[value & 15];
		string[3] = '\'';
		string[4] = ' ';

		globals_list = emit(string, globals_list);
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
			globals_list = emit("%0 ", globals_list);
		}
	}

}

void global_pad_to_register_size(int bytes_written)
{
	int alignment_size = register_size - (bytes_written % register_size);
	while(alignment_size != 0)
	{
		globals_list = emit("'00' ", globals_list);

		alignment_size = alignment_size - 1;
	}
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
	maybe_bootstrap_error("global array definitions");

	global_variable_header(name);

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

	require_match("missing ;\n", ";");

	return array_modifier;
}

void global_variable_definition(struct type* type_size, char* variable_name)
{
	global_variable_header(variable_name);

	global_variable_zero_initialize(type_size->size);

	require_extra_token();
}

void global_assignment(char* name, struct type* type_size)
{
	global_variable_header(name);

	require_extra_token();

	global_value_selection(type_size);

	global_pad_to_register_size(type_size->size);

	require_match("ERROR in Program\nMissing ;\n", ";");
}

void declare_global_variable(struct type* type_size, char* name)
{
	/* Deal with global static arrays */
	if(match("[", global_token->s))
	{
		global_symbol_list->array_modifier = global_static_array(type_size, name);
		return;
	}

	/* Deal with global variables */
	if(match(";", global_token->s))
	{
		global_variable_definition(type_size, name);
		return;
	}

	/* Deal with assignment to a global variable */
	if(match("=", global_token->s))
	{
		global_assignment(name, type_size);
		return;
	}
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

new_type:
	/* Deal with garbage input */
	if (NULL == global_token) return;
	require('#' != global_token->s[0], "unhandled macro directive\n");
	require(!match("\n", global_token->s), "unexpected newline token\n");

	/* Handle cc_* CONSTANT statements */
	if(match("CONSTANT", global_token->s))
	{
		global_constant();
		goto new_type;
	}

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
	if(global_token->s[0] == '(')
	{
		require_extra_token(); /* skip '(' */
		require_match("Required '*' after '*' in global function pointer.\n", "*");

		name = global_token->s;
		require_extra_token();

		require_match("Required ')' after name in global function pointer.\n", ")");
		require_match("Required '(' after ')' in global function pointer.\n", "(");

		while(global_token->s[0] != ')')
		{
			type_name();

			if(global_token->s[0] == ',')
			{
				require_extra_token();
			}
		}
		require_extra_token(); /* skip ')' */

		type_size = function_pointer;
	}
	else
	{
		name = global_token->s;
		require_extra_token();
	}

	if(global_token->s[0] == ';' || global_token->s[0] == '=' || global_token->s[0] == '[')
	{
		global_symbol_list = sym_declare(name, type_size, global_symbol_list);

		declare_global_variable(type_size, name);
		goto new_type;
	}

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
