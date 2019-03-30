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

// CONSTANT stdin 0
// CONSTANT stdout 1
// CONSTANT stderr 2
// CONSTANT EOF 0xFFFFFFFF

int fgetc(FILE* f)
{
	asm("!4 R0 SUB R12 ARITH_ALWAYS"
	    "!0 R0 LOAD32 R0 MEMORY"
	    "{R0} PUSH_ALWAYS"
	    "'0' SP R1 NO_SHIFT MOVE_ALWAYS"
	    "!1 R2 LOADI8_ALWAYS"
	    "!3 R7 LOADI8_ALWAYS"
	    "SYSCALL_ALWAYS"
	    "!0 CMPI8 R0 IMM_ALWAYS"
	    "{R0} POP_ALWAYS"
	    "!0 R0 MVNI8_EQUAL");
}

void fputc(char s, FILE* f)
{
	asm("!8 R0 SUB R12 ARITH_ALWAYS"
	    "!0 R0 LOAD32 R0 MEMORY"
	    "!4 R1 SUB R12 ARITH_ALWAYS"
	    "!1 R2 LOADI8_ALWAYS"
	    "!4 R7 LOADI8_ALWAYS"
	    "SYSCALL_ALWAYS");
}

/* Important values needed for open
 * O_RDONLY => 0
 * O_WRONLY => 1
 * O_RDWR => 2
 * O_CREAT => 64
 * O_TRUNC => 512
 * S_IRWXU => 00700
 * S_IXUSR => 00100
 * S_IWUSR => 00200
 * S_IRUSR => 00400
 */

FILE* open(char* name, int flag, int mode)
{
	asm("!4 R0 SUB R12 ARITH_ALWAYS"
	    "!0 R0 LOAD32 R0 MEMORY"
	    "!8 R1 SUB R12 ARITH_ALWAYS"
	    "!0 R1 LOAD32 R1 MEMORY"
	    "!12 R2 SUB R12 ARITH_ALWAYS"
	    "!0 R2 LOAD32 R2 MEMORY"
	    "!5 R7 LOADI8_ALWAYS"
	    "SYSCALL_ALWAYS");
}

FILE* fopen(char* filename, char* mode)
{
	FILE* f;
	if('w' == mode[0])
	{ /* 577 is O_WRONLY|O_CREAT|O_TRUNC, 384 is 600 in octal */
		f = open(filename, 577 , 384);
	}
	else
	{ /* Everything else is a read */
		f = open(filename, 0, 0);
	}

	/* Negative numbers are error codes */
	if(0 > f)
	{
		return 0;
	}
	return f;
}

int close(int fd)
{
	asm("!4 R0 SUB R12 ARITH_ALWAYS"
	    "!6 R7 LOADI8_ALWAYS"
	    "SYSCALL_ALWAYS");
}
int fclose(FILE* stream)
{
	int error = close(stream);
	return error;
}
