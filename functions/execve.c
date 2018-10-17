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

void exit(int value);

void _exit(int value)
{
	exit(value);
}

int waitpid (int pid, int* status_ptr, int options)
{
	asm("LOAD_EFFECTIVE_ADDRESS_ebx %12"
	"LOAD_INTEGER_ebx"
	"LOAD_EFFECTIVE_ADDRESS_ecx %8"
	"LOAD_INTEGER_ecx"
	"LOAD_EFFECTIVE_ADDRESS_edx %4"
	"LOAD_INTEGER_edx"
	"LOAD_IMMEDIATE_eax %7"
	"INT_80");
}


int execve(char* file_name, char** argv, char** envp)
{
	asm("LOAD_EFFECTIVE_ADDRESS_ebx %12"
	"LOAD_INTEGER_ebx"
	"LOAD_EFFECTIVE_ADDRESS_ecx %8"
	"LOAD_INTEGER_ecx"
	"LOAD_EFFECTIVE_ADDRESS_edx %4"
	"LOAD_INTEGER_edx"
	"LOAD_IMMEDIATE_eax %11"
	"INT_80");
}
