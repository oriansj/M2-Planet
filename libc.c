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
// CONSTANT NULL 0
// CONSTANT EXIT_FAILURE 1
// CONSTANT EXIT_SUCCESS 0
// CONSTANT stdin 0
// CONSTANT stdout 1
// CONSTANT stderr 2

int fgetc(FILE* f);
void fputc(char s, FILE* f);
FILE* fopen(char* filename, char* mode);
int fclose(FILE* file);
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
		":FUNCTION_malloc_Done"
		"RETURN");
}
void* calloc(int count, int size)
{
	char* ret = malloc(count * size);
	int i;
	for(i = (count * size); i >= 0; i = i - 1)
	{
		ret[i] = 0;
	}
	return ret;
}

void free(void* pointer)
{
	return;
}
void exit(int value)
{
	asm("POP_ebx"
		"POP_ebx"
		"LOAD_IMMEDIATE_eax %1"
		"INT_80");
}
