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

typedef union { int seven; int also_seven; int also_also_seven; } SevenUnion;

struct {
	int one;
	struct {
		int two;
		int three;
	};
	int four;
} s;

struct {
	int one;
	union {
		int two;
		int also_two;
	};
	int four;
} su;

int main() {
	s.one = 1;
	s.two = 2;
	s.three = 3;
	s.four = 4;

	if(s.one != 1) return 1;
	if(s.two != 2) return 2;
	if(s.three != 3) return 3;
	if(s.four != 4) return 4;

	if(sizeof(s) != 4 * sizeof(int)) return 5;

	su.one = 1;
	su.two = 2;
	su.four = 4;

	if(su.one != 1) return 6;
	if(su.two != 2) return 6;
	if(su.also_two != 2) return 7; /* Undefined behavior, but we can still use it to test behavior in M2-Planet */
	if(su.four != 4) return 8;

	if(sizeof(su) != 3 * sizeof(int)) return 9;

	union {
		int six;
		int also_six;
		int also_also_six;
	} t;

	t.six = 6;
	if(t.six != 6) return 10;
	if(t.also_six != 6) return 11;
	if(t.also_also_six != 6) return 12;

	SevenUnion seven;
	seven.seven = 7;
	if(seven.seven != 7) return 13;
	if(seven.also_seven != 7) return 14;
	if(seven.also_also_seven != 7) return 15;
}

