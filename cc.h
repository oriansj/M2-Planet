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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define GLOBAL 1
#define FUNCTION 2
#define LOCAL_VARIABLE 4
#define ARGUEMENT 8

struct token_list
{
	struct token_list* next;
	union
	{
		struct token_list* prev;
		struct token_list* entry;
	};
	char* s;
	union
	{
		int length;
		int type;
		bool hands_off;
	};
};

struct token_list* global_token;
