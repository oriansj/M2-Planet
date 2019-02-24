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
	asm("STORE_eax_into_ESP_IMMEDIATE8 !4"
	"PUSH_eax"
	"LOAD_IMMEDIATE_eax %45"
	"LOAD_IMMEDIATE_ebx %0"
	"INT_80"
	"POP_ebx"
	"ADD_eax_to_ebx"
	"PUSH_eax"
	"PUSH_ebx"
	"LOAD_IMMEDIATE_eax %45"
	"INT_80"
	"POP_ebx"
	"CMP"
	"POP_eax"
	"JUMP_EQ8 !FUNCTION_malloc_Done"
	"LOAD_IMMEDIATE_eax %-1"
	":FUNCTION_malloc_Done");
}
