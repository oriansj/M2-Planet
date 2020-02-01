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

// CONSTANT stdin 0x1100
// CONSTANT stdout 0x1101
// CONSTANT stderr 0
// CONSTANT EOF 0xFFFFFFFF

int match(char* a, char* b);

int fgetc(FILE* f)
{
	asm("LOAD R1 R14 0"
	    "FGETC");
}

void fputc(char s, FILE* f)
{
	asm("LOAD R0 R14 0"
	    "LOAD R1 R14 4"
	    "FPUTC");
}

FILE* open_read(int filename)
{
	asm("LOAD R0 R14 0"
	    "FOPEN_READ");
}

FILE* open_write(int filename)
{
	asm("LOAD R0 R14 0"
	    "FOPEN_WRITE");
}

int fclose(FILE* stream)
{
	asm("LOAD R0 R14 0"
	    "FCLOSE");
}

FILE* fopen(char* filename, char* mode)
{
	FILE* f;
	int fd = 0;
	if(match(filename, "STDIN") || match(filename, "tape_01"))
	{
		fd = 0x1100;
	}
	else if(match(filename, "STDOUT") || match(filename, "tape_02"))
	{
		fd = 0x1101;
	}

	if('w' == mode[0])
	{ /* 577 is O_WRONLY|O_CREAT|O_TRUNC, 384 is 600 in octal */
		f = open_write(fd);
	}
	else
	{ /* Everything else is a read */
		f = open_read(fd);
	}

	/* Negative numbers are error codes */
	if(0 > f)
	{
		return 0;
	}
	return f;
}
