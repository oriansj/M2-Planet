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
#include <stdint.h>

/* Initialize default types */
void initialize_types()
{
	/* Define void */
	global_types = calloc(1, sizeof(struct type));
	global_types->name = "void";
	global_types->size = 4;
	global_types->type = global_types;
	/* void* has the same properties as void */
	global_types->indirect = global_types;

	/* Define int */
	struct type* a = calloc(1, sizeof(struct type));
	a->name = "int";
	a->size = 4;
	/* int* has the same properties as int */
	a->indirect = a;
	a->type = a;

	/* Define char* */
	struct type* b = calloc(1, sizeof(struct type));
	b->name = "char*";
	b->size = 4;
	b->type = b;

	/* Define char */
	struct type* c = calloc(1, sizeof(struct type));
	c->name = "char";
	c->size = 1;
	c->type = c;

	/* char** is char */
	c->indirect = b;
	b->indirect = c;

	/* Define FILE */
	struct type* d = calloc(1, sizeof(struct type));
	d->name = "FILE";
	d->size = 4;
	d->type = d;
	/* int* has the same properties as int */
	d->indirect = d;

	/* Finalize type list */
	c->next = d;
	a->next = c;
	global_types->next = a;
}
