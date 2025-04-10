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
}

