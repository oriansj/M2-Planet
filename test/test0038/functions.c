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

static int a;

static int add_static(int a) { return a + a; }

inline int add_inline(int c) { return c + c; }

static inline int add_both(int x) { return x + x; }

inline static int add_reversed(int b) { return b + b; }

_Noreturn void do_not_return();

int main() {
	a = 1;

	if(2 != add_static(1)) return 1;
	if(2 != add_both(1)) return 2;
	if(2 != add_reversed(1)) return 3;
	if(2 != add_inline(1)) return 4;

	if(a != 1) return 5;

	return 0;
}

