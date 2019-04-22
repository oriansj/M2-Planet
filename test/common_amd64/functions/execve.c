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
	/* Uses wait4 with struct rusage *ru set to NULL */
	asm("LOAD_EFFECTIVE_ADDRESS_rdi %24"
	"LOAD_INTEGER_rdi"
	"LOAD_EFFECTIVE_ADDRESS_rsi %16"
	"LOAD_INTEGER_rsi"
	"LOAD_EFFECTIVE_ADDRESS_rdx %8"
	"LOAD_INTEGER_rdx"
	"LOAD_IMMEDIATE_r10 %0"
	"LOAD_IMMEDIATE_rax %61"
	"SYSCALL");
}


int execve(char* file_name, char** argv, char** envp)
{
	asm("LOAD_EFFECTIVE_ADDRESS_rdi %24"
	"LOAD_INTEGER_rdi"
	"LOAD_EFFECTIVE_ADDRESS_rsi %16"
	"LOAD_INTEGER_rsi"
	"LOAD_EFFECTIVE_ADDRESS_rdx %8"
	"LOAD_INTEGER_rdx"
	"LOAD_IMMEDIATE_rax %59"
	"SYSCALL");
}
