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

int getchar()
{
	asm("LOAD_IMMEDIATE_rdi %0"
	"LOAD_IMMEDIATE_rax %0"
	"PUSH_RAX"
	"LOAD_EFFECTIVE_ADDRESS_rsi %0"
	"LOAD_IMMEDIATE_rdx %1"
	"SYSCALL"
	"LOAD_IMMEDIATE_rbx %0"
	"CMP"
	"POP_RAX"
	"JUMP_NE %FUNCTION_getchar_Done"
	"LOAD_IMMEDIATE_rax %-1"
	":FUNCTION_getchar_Done");
}
