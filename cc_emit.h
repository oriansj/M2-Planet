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

#include "cc.h"

struct token_list* emit(char *s, struct token_list* head);
void emit_out(char* s);

void emit_label(char* prefix, char* name);

extern char* emit_string;
void emit_to_string(char* s);
void reset_emit_string(void);

char* register_from_string(int reg);

void emit_unconditional_jump(char* prefix, char* name, char* note);

void emit_jump_if_zero(int reg, char* prefix, char* name, char* note);

void emit_load_named_immediate(int reg, char* prefix, char* name, char* note);

void write_load_immediate(int reg, int value, char* note);

void emit_load_immediate(int reg, int value, char* note);

/* Adds destination and source and places result in destination */
void write_add(int destination_reg, int source_reg, char* note);

void emit_add(int destination_reg, int source_reg, char* note);

void write_add_immediate(int reg, int value, char* note);

void emit_add_immediate(int reg, int value, char* note);

/* Subtracts destination and source and places result in destination */
void write_sub(int destination_reg, int source_reg, char* note);

void emit_sub(int destination_reg, int source_reg, char* note);

void write_sub_immediate(int reg, int value, char* note);

void emit_sub_immediate(int reg, int value, char* note);

void emit_mul_into_register_zero(int reg, char* note);

void emit_mul_register_zero_with_immediate(int value, char* note);

void write_move(int destination_reg, int source_reg, char* note);

void emit_move(int destination_reg, int source_reg, char* note);

void emit_load_relative_to_register(int destination, int offset_register, int value, char* note);

void emit_dereference(int reg, char* note);

void emit_push(int reg, char* note);

void emit_pop(int reg, char* note);

