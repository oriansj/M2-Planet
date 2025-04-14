/* Copyright (C) 2024 Gtker
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

int global[2] = { 1, 2 };

struct T {
	struct {
		short i;
	} c;
} global_t;

int main() {
    unsigned a = 0;
    int b = (int)a;

    if(b != 0) return 1;

	void* func = 0;
	void(*caller)(void) = (void (*)(void))func;

	void* void_ptr = global;

	int r = ((int*)(void_ptr))[0];
	if(r != 1) return 2;

	r = ((int*)(void_ptr))[1];
	if(r != 2) return 3;

	int a = 0;
	void* void_a = (void*)&a;

	*(int*)void_a = 1;

	if(a != 1) return 4;

	int b = *(int*)void_a;

	if(b != 1) return 5;

	*(int*)void_a = 0;

	if(a != 0) return 6;

	b = *(int*)void_a;

	if(b != 0) return 7;

	short s = 12;
	void* void_s = (void*)&s;
	struct T* local_t = &global_t;

	*(short*)void_s = local_t->c.i;

	if(s != 0) return 8;
}

