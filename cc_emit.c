/* Copyright (C) 2025 Gtker
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

void emit_load_named_immediate(int reg, char* prefix, char* name, char* note);

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

void emit_label(char* prefix, char* name)
{
	emit_out(":");
	emit_out(prefix);
	emit_out(name);
	emit_out("\n");
}

char* emit_string;
int emit_string_index;

void emit_to_string(char* s)
{
	emit_string_index = emit_string_index + copy_string(emit_string + emit_string_index, s, MAX_STRING - emit_string_index);
}

void reset_emit_string(void)
{
	/* Emitted strings are just added to a linked list so we need a new one each time. */
	emit_string = calloc(MAX_STRING, sizeof(char));
	emit_string_index = 0;
}

char* register_from_string(int reg)
{
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		if(reg == REGISTER_ZERO) return "0";
		else if(reg == REGISTER_ONE) return "1";
		else if(reg == REGISTER_EMIT_TEMP) return "10";
		else if(reg == REGISTER_UNUSED2) return "11";
		else if(reg == REGISTER_LOCALS) return "12";
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
		else if(reg == REGISTER_EMIT_TEMP) return "ecx";
		else if(reg == REGISTER_UNUSED2) return "edx";
	}
	else if(AMD64 == Architecture)
	{
		if(reg == REGISTER_ZERO) return "rax";
		else if(reg == REGISTER_ONE) return "rbx";
		else if(reg == REGISTER_TEMP) return "rdi";
		else if(reg == REGISTER_BASE) return "rbp";
		else if(reg == REGISTER_STACK) return "rsp";
		else if(reg == REGISTER_LOCALS) return "r13";
		else if(reg == REGISTER_EMIT_TEMP) return "r14";
		else if(reg == REGISTER_UNUSED2) return "r15";
	}
	else if(ARMV7L == Architecture)
	{
		if(reg == REGISTER_ZERO) return "R0";
		else if(reg == REGISTER_ONE) return "R1";
		else if(reg == REGISTER_LOCALS) return "R8";
		else if(reg == REGISTER_EMIT_TEMP) return "R9";
		else if(reg == REGISTER_UNUSED2) return "R10";
		else if(reg == REGISTER_TEMP) return "R11";
		else if(reg == REGISTER_BASE) return "BP";
		else if(reg == REGISTER_RETURN) return "LR";
		else if(reg == REGISTER_STACK) return "SP";
	}
	else if(AARCH64 == Architecture)
	{
		if(reg == REGISTER_ZERO) return "X0";
		else if(reg == REGISTER_ONE) return "X1";
		else if(reg == REGISTER_LOCALS) return "X13";
		else if(reg == REGISTER_EMIT_TEMP) return "X14";
		else if(reg == REGISTER_UNUSED2) return "X15";
		else if(reg == REGISTER_TEMP) return "X16";
		else if(reg == REGISTER_BASE) return "BP";
		else if(reg == REGISTER_RETURN) return "LR";
		else if(reg == REGISTER_STACK) return "SP";
	}
	else if(RISCV32 == Architecture || RISCV64 == Architecture)
	{
		if(reg == REGISTER_ZERO) return "a0";
		else if(reg == REGISTER_ONE) return "a1";
		else if(reg == REGISTER_LOCALS) return "t3";
		else if(reg == REGISTER_EMIT_TEMP) return "t4";
		else if(reg == REGISTER_UNUSED2) return "t5";
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
void emit_unconditional_jump(char* prefix, char* name, char* note)
{
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		emit_out("JUMP @");
		emit_out(prefix);
		emit_out(name);
	}
	else if((X86 == Architecture) || (AMD64 == Architecture))
	{
		emit_out("jmp %");
		emit_out(prefix);
		emit_out(name);
	}
	else if(ARMV7L == Architecture)
	{
		emit_out("^~");
		emit_out(prefix);
		emit_out(name);
		emit_out(" JUMP_ALWAYS");
	}
	else if(AARCH64 == Architecture)
	{
		emit_load_named_immediate(REGISTER_TEMP, prefix, name, note);
		emit_out("BR_X16");
	}
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
	{
		emit_out("$");
		emit_out(prefix);
		emit_out(name);
		emit_out(" jal");
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

void emit_jump_if_zero(int reg, char* prefix, char* name, char* note)
{
	char* reg_name = register_from_string(reg);

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		emit_out("JUMP.Z R");
		emit_out(reg_name);
		emit_out(" @");
		emit_out(prefix);
		emit_out(name);
	}
	else if((X86 == Architecture) || (AMD64 == Architecture))
	{
		emit_out("test_");
		emit_out(reg_name);
		emit_out(",");
		emit_out(reg_name);
		emit_out("\nje %");
		emit_out(prefix);
		emit_out(name);
	}
	else if(ARMV7L == Architecture)
	{
		emit_out("!0 CMPI8 ");
		emit_out(reg_name);
		emit_out(" IMM_ALWAYS\n^~");
		emit_out(prefix);
		emit_out(name);
		emit_out(" JUMP_EQUAL");
	}
	else if(AARCH64 == Architecture)
	{
		emit_out("CBNZ_");
		emit_out(reg_name);
		emit_out("_PAST_BR\n");
		emit_load_named_immediate(REGISTER_TEMP, prefix, name, note);
		emit_out("BR_X16");
	}
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
	{
		emit_out("rs1_");
		emit_out(reg_name);
		emit_out(" @8 bnez\n$");
		emit_out(prefix);
		emit_out(name);
		emit_out(" jal");
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

void write_load_immediate(int reg, int value, char* note)
{
	char* reg_name = register_from_string(reg);
	char* value_string = int2str(value, 10, TRUE);
	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		if((32767 > value) && (value > -32768))
		{
			emit_to_string("LOADI R");
			emit_to_string(reg_name);
			emit_to_string(" ");
			emit_to_string(value_string);
		}
		else
		{
			emit_to_string("LOADR R");
			emit_to_string(reg_name);
			emit_to_string(" 4\nJUMP 4\n%");
			emit_to_string(int2str(value, 10, TRUE));
		}
	}
	else if(X86 == Architecture || AMD64 == Architecture)
	{
		if(value == 0)
		{
			/* This is the recommended way of zeroing a register on x86/amd64.
			 * xor eax, eax (32 bit registers) for both x86 and amd64 since it
			 * takes up a byte less and still zeros the register. */
			emit_to_string("xor_e");
			/* amd64 register starts with r but we need it to start with e */
			emit_to_string(reg_name + 1);
			emit_to_string(",e");
			emit_to_string(reg_name + 1);
		}
		else
		{
			emit_to_string("mov_");
			emit_to_string(reg_name);
			emit_to_string(", %");
			emit_to_string(value_string);
		}
	}
	else if(ARMV7L == Architecture)
	{
		if((127 >= value) && (value >= 0))
		{
			emit_to_string("!");
			emit_to_string(value_string);
			emit_to_string(" ");
			emit_to_string(reg_name);
			emit_to_string(" LOADI8_ALWAYS");
		}
		else
		{
			emit_to_string("!0 ");
			emit_to_string(reg_name);
			emit_to_string(" LOAD32 R15 MEMORY\n~0 JUMP_ALWAYS\n%");
			emit_to_string(value_string);
		}
	}
	else if(AARCH64 == Architecture)
	{
		if((value == 0 && (reg == 0 || reg == 1)) || (value == 1 && reg == 0))
		{
			emit_to_string("SET_");
			emit_to_string(reg_name);
			emit_to_string("_TO_");
			emit_to_string(value_string);
		}
		else
		{
			emit_to_string("LOAD_W");
			/* Normal register starts with X for 64bit wide
			 * but we need W. */
			emit_to_string(reg_name + 1);
			emit_to_string("_AHEAD\nSKIP_32_DATA\n%");
			emit_to_string(value_string);
		}
	}
	else if((RISCV32 == Architecture) || (RISCV64 == Architecture))
	{
		if((2047 >= value) && (value >= -2048))
		{
			emit_to_string("rd_");
			emit_to_string(reg_name);
			emit_to_string(" !");
			emit_to_string(value_string);
			emit_to_string(" addi");
		}
		else if (0 == (value >> 30))
		{
			emit_to_string("rd_");
			emit_to_string(reg_name);
			emit_to_string(" ~");
			emit_to_string(value_string);
			emit_to_string(" lui\n");

			emit_to_string("rd_");
			emit_to_string(reg_name);
			emit_to_string(" rs1_");
			emit_to_string(reg_name);
			emit_to_string(" !");
			emit_to_string(value_string);
			emit_to_string(" addi");
		}
		else
		{
			int high = value >> 30;
			char* high_string = int2str(high, 10, TRUE);
			int low = ((value >> 30) << 30) ^ value;
			char* low_string = int2str(low, 10, TRUE);

			emit_to_string("rd_");
			emit_to_string(reg_name);
			emit_to_string(" ~");
			emit_to_string(high_string);
			emit_to_string(" lui\n");

			emit_to_string("rd_");
			emit_to_string(reg_name);
			emit_to_string(" rs1_");
			emit_to_string(reg_name);
			emit_to_string(" !");
			emit_to_string(high_string);
			emit_to_string(" addi\n");

			emit_to_string("rd_");
			emit_to_string(reg_name);
			emit_to_string(" rs1_");
			emit_to_string(reg_name);
			emit_to_string(" rs2_x30 slli\n");

			emit_to_string("rd_t1 ~");
			emit_to_string(low_string);
			emit_to_string(" lui\n");

			emit_to_string("rd_t1 rs1_t1 !");
			emit_to_string(low_string);
			emit_to_string(" addi\n");

			emit_to_string("rd_");
			emit_to_string(reg_name);
			emit_to_string(" rs1_");
			emit_to_string(reg_name);
			emit_to_string(" rs2_t1 or");
		}
	}

	if(note == NULL)
	{
		emit_to_string("\n");
	}
	else
	{
		emit_to_string(" # ");
		emit_to_string(note);
		emit_to_string("\n");
	}
}

void emit_load_immediate(int reg, int value, char* note)
{
	reset_emit_string();
	write_load_immediate(reg, value, note);
	emit_out(emit_string);
}

/* Adds destination and source and places result in destination */
void write_add(int destination_reg, int source_reg, char* note)
{
	char* destination_name = register_from_string(destination_reg);
	char* source_name = register_from_string(source_reg);

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		emit_to_string("ADD R");
		emit_to_string(destination_name);
		emit_to_string(" R");
		emit_to_string(destination_name);
		emit_to_string(" R");
		emit_to_string(source_name);
	}
	else if(X86 == Architecture || AMD64 == Architecture)
	{
		emit_to_string("add_");
		emit_to_string(destination_name);
		emit_to_string(",");
		emit_to_string(source_name);
	}
	else if(ARMV7L == Architecture)
	{
		emit_to_string("'0' ");
		emit_to_string(destination_name);
		emit_to_string(" ");
		emit_to_string(destination_name);
		emit_to_string(" ADD ");
		emit_to_string(source_name);
		emit_to_string(" ARITH2_ALWAYS");
	}
	else if(AARCH64 == Architecture)
	{
		emit_to_string("ADD_");
		emit_to_string(destination_name);
		emit_to_string("_");
		emit_to_string(source_name);
		emit_to_string("_");
		emit_to_string(destination_name);
	}
	else if(RISCV32 == Architecture || RISCV64 == Architecture)
	{
		emit_to_string("rd_");
		emit_to_string(destination_name);
		emit_to_string(" rs1_");
		emit_to_string(source_name);
		emit_to_string(" rs2_");
		emit_to_string(destination_name);
		emit_to_string(" add");
	}

	if(note == NULL)
	{
		emit_to_string("\n");
	}
	else
	{
		emit_to_string(" # ");
		emit_to_string(note);
		emit_to_string("\n");
	}
}

void emit_add(int destination_reg, int source_reg, char* note)
{
	reset_emit_string();
	write_add(destination_reg, source_reg, note);
	emit_out(emit_string);
}

void write_add_immediate(int reg, int value, char* note)
{
	write_load_immediate(REGISTER_EMIT_TEMP, value, note);
	write_add(reg, REGISTER_EMIT_TEMP, note);
}

void emit_add_immediate(int reg, int value, char* note)
{
	reset_emit_string();
	write_add_immediate(reg, value, note);
	emit_out(emit_string);
}

/* Subtracts destination and source and places result in destination */
void write_sub(int destination_reg, int source_reg, char* note)
{
	char* destination_name = register_from_string(destination_reg);
	char* source_name = register_from_string(source_reg);

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		emit_to_string("SUB R");
		emit_to_string(destination_name);
		emit_to_string(" R");
		emit_to_string(destination_name);
		emit_to_string(" R");
		emit_to_string(source_name);
	}
	else if(X86 == Architecture || AMD64 == Architecture)
	{
		emit_to_string("sub_");
		emit_to_string(destination_name);
		emit_to_string(",");
		emit_to_string(source_name);
		emit_to_string("\n");
	}
	else if(ARMV7L == Architecture)
	{
		emit_to_string("'0' ");
		emit_to_string(source_name);
		emit_to_string(" ");
		emit_to_string(destination_name);
		emit_to_string(" SUB ");
		emit_to_string(destination_name);
		emit_to_string(" ARITH2_ALWAYS");
	}
	else if(AARCH64 == Architecture)
	{
		emit_to_string("SUB_");
		emit_to_string(destination_name);
		emit_to_string("_");
		emit_to_string(destination_name);
		emit_to_string("_");
		emit_to_string(source_name);
	}
	else if(RISCV32 == Architecture || RISCV64 == Architecture)
	{
		emit_to_string("rd_");
		emit_to_string(destination_name);
		emit_to_string(" rs1_");
		emit_to_string(destination_name);
		emit_to_string(" rs2_");
		emit_to_string(source_name);
		emit_to_string(" sub");
	}

	if(note == NULL)
	{
		emit_to_string("\n");
	}
	else
	{
		emit_to_string(" # ");
		emit_to_string(note);
		emit_to_string("\n");
	}
}

void emit_sub(int destination_reg, int source_reg, char* note)
{
	reset_emit_string();
	write_sub(destination_reg, source_reg, note);
	emit_out(emit_string);
}

void write_sub_immediate(int reg, int value, char* note)
{
	write_load_immediate(REGISTER_EMIT_TEMP, value, note);
	write_sub(reg, REGISTER_EMIT_TEMP, note);
}

void emit_sub_immediate(int reg, int value, char* note)
{
	reset_emit_string();
	write_sub_immediate(reg, value, note);
	emit_out(emit_string);
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
	emit_load_immediate(REGISTER_EMIT_TEMP, value, note);
	emit_mul_into_register_zero(REGISTER_EMIT_TEMP, note);
}

void write_move(int destination_reg, int source_reg, char* note)
{
	char* destination_name = register_from_string(destination_reg);
	char* source_name = register_from_string(source_reg);

	if((KNIGHT_POSIX == Architecture) || (KNIGHT_NATIVE == Architecture))
	{
		emit_to_string("COPY R");
		emit_to_string(destination_name);
		emit_to_string(" R");
		emit_to_string(source_name);
	}
	else if(X86 == Architecture || AMD64 == Architecture)
	{
		emit_to_string("mov_");
		emit_to_string(destination_name);
		emit_to_string(",");
		emit_to_string(source_name);
	}
	else if(ARMV7L == Architecture)
	{
		emit_to_string("'0' ");
		emit_to_string(source_name);
		emit_to_string(" ");
		emit_to_string(destination_name);
		emit_to_string(" NO_SHIFT MOVE_ALWAYS");
	}
	else if(AARCH64 == Architecture)
	{
		emit_to_string("SET_");
		emit_to_string(destination_name);
		emit_to_string("_FROM_");
		emit_to_string(source_name);
	}
	else if(RISCV32 == Architecture || RISCV64 == Architecture)
	{
		emit_to_string("rd_");
		emit_to_string(destination_name);
		emit_to_string(" rs1_");
		emit_to_string(source_name);
		emit_to_string(" mv");
	}

	if(note == NULL)
	{
		emit_to_string("\n");
	}
	else
	{
		emit_to_string(" # ");
		emit_to_string(note);
		emit_to_string("\n");
	}
}

void emit_move(int destination_reg, int source_reg, char* note)
{
	reset_emit_string();
	write_move(destination_reg, source_reg, note);
	emit_out(emit_string);
}

void emit_load_relative_to_register(int destination, int offset_register, int value, char* note)
{
	char* destination_name = register_from_string(destination);
	char* offset_name = register_from_string(offset_register);
	char* value_string = int2str(value, 10, TRUE);
	int absolute_value = value;
	if(value < 0)
	{
		absolute_value = -absolute_value;
	}

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
		if((127 >= absolute_value) && (absolute_value >= 0))
		{
			emit_out("!");
			emit_out(int2str(absolute_value, 10, FALSE));
			emit_out(" ");
			emit_out(destination_name);
			emit_out(" SUB ");
			emit_out(offset_name);
			emit_out(" ARITH_ALWAYS\n");
		}
		else
		{
			emit_move(destination, offset_register, note);
			emit_sub_immediate(destination, absolute_value, note);
		}
	}
	else if(AARCH64 == Architecture)
	{
		emit_move(destination, offset_register, note);
		emit_sub_immediate(destination, absolute_value, note);
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


