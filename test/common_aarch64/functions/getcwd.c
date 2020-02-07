/* Copyright (C) 2020 Jeremiah Orians
 * Copyright (C) 2020 deesix <deesix@tuta.io>
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

#include<stdlib.h>
//CONSTANT PATH_MAX 4096
#define PATH_MAX 4096

int _getcwd(char* buf, size_t size)
{
	asm("SET_X0_FROM_BP" "SUB_X0_16" "DEREF_X0"
	    "SET_X1_FROM_X0"
	    "SET_X0_FROM_BP" "SUB_X0_8" "DEREF_X0"
	    "SET_X8_TO_SYS_GETCWD"
	    "SYSCALL");
}

char* getcwd(char* buf, size_t size)
{
	int c = _getcwd(buf, size);
	if(0 == c) return NULL;
	return buf;
}

char* getwd(char* buf)
{
	return getcwd(buf, PATH_MAX);
}

char* get_current_dir_name()
{
	return getcwd(malloc(PATH_MAX), PATH_MAX);
}
