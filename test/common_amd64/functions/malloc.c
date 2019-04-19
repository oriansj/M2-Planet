/* Copyright (C) 2016 Jeremiah Orians
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

// CONSTANT NULL 0

void* malloc(int size)
{
	asm("LOAD_RSP_IMMEDIATE_into_rax %8"
	"PUSH_RAX"
	"LOAD_IMMEDIATE_rax %12"
	"LOAD_IMMEDIATE_rdi %0"
	"SYSCALL"
	"POP_RBX"
	"ADD_rax_to_rbx"
	"COPY_rbx_to_rdi"
	"PUSH_RAX"
	"PUSH_RBX"
	"LOAD_IMMEDIATE_rax %12"
	"SYSCALL"
	"POP_RBX"
	"CMP"
	"POP_RAX"
	"JUMP_EQ %FUNCTION_malloc_Done"
	"LOAD_IMMEDIATE_rax %-1"
	":FUNCTION_malloc_Done");
}
