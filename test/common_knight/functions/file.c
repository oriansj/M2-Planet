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
	asm("LOAD R1 R14 0"
	    "FGETC");
}

void fputc(char s, FILE* f)
{
	asm("LOAD R0 R14 0"
	    "LOAD R1 R14 4"
	    "FPUTC");
}

FILE* open_write(char* filename)
{
	asm("LOAD R0 R14 0"
	    "FOPEN_WRITE");
}

FILE* open_read(char* filename)
{
	asm("LOAD R0 R14 0"
	    "FOPEN_READ");
}

FILE* fopen(char* filename, char* mode)
{
	FILE* f;
	if('w' == mode[0])
	{
		f = open_write(filename);
	}
	else
	{ /* Everything else is a read */
		f = open_read(filename);
	}

	/* Negative numbers are error codes */
	if(0 > f)
	{
		return 0;
	}
	return f;
}

int fclose(FILE* stream)
{
	asm("LOAD R0 R14 0"
	    "FCLOSE");
}
