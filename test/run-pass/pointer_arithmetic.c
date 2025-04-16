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

int test_ints(int* i) {
	if (*i != 0xFF) {
		return 1;
	}

	i += 1;

	if (*i != 0xFE) {
		return 1;
	}

	i += 2;

	if (*i != 0xFC) {
		return 1;
	}

	i -= 2;

	if (*i != 0xFE) {
		return 1;
	}

	i -= 1;

	if (*i != 0xFF) {
		return 1;
	}

	i[0] += 2;

	if (*i != 0x101) {
		return 1;
	}

	i[0] -= 2;

	if (*i != 0xFF) {
		return 1;
	}

	*i += 2;

	if (*i != 0x101) {
		return 1;
	}

	*i -= 2;

	if (*i != 0xFF) {
		return 1;
	}

	int one = 1;
	int two = 2;

	i += one;

	if (*i != 0xFE) {
		return 1;
	}

	i += two;

	if (*i != 0xFC) {
		return 1;
	}

	i -= two;

	if (*i != 0xFE) {
		return 1;
	}

	i -= one;

	if (*i != 0xFF) {
		return 1;
	}

	i[0] += two;

	if (*i != 0x101) {
		return 1;
	}

	i[0] -= two;

	if (*i != 0xFF) {
		return 1;
	}

	*i += two;

	if (*i != 0x101) {
		return 1;
	}

	*i -= two;

	if (*i != 0xFF) {
		return 1;
	}

	return 0;
}

int test_chars(char* c) {
	if(*c != 0){
		return 1;
	}

	c += 1;

	if (*c != 1) {
		return 1;
	}

	c += 2;

	if(*c != 3) {
		return 1;
	}

	c -= 2;

	if(*c != 1) {
		return 1;
	}

	c -= 1;

	if(*c != 0) {
		return 1;
	}

	int one = 1;
	int two = 2;

	c += one;

	if (*c != 1) {
		return 1;
	}

	c += two;

	if(*c != 3) {
		return 1;
	}

	c -= two;

	if(*c != 1) {
		return 1;
	}

	c -= one;

	if(*c != 0) {
		return 1;
	}

	return 0;
}

struct Struct {
	int a;
	int b;
	int c;
};

int test_struct(struct Struct* s) {
	if(s->a != 1 || s->b != 2 || s->c != 3) {
		return 1;
	}

	s += 1;

	if(s->a != 2 || s->b != 3 || s->c != 4) {
		return 1;
	}

	s += 2;

	if(s->a != 4 || s->b != 5 || s->c != 6) {
		return 1;
	}

	s -= 2;

	if(s->a != 2 || s->b != 3 || s->c != 4) {
		return 1;
	}

	s -= 1;

	if(s->a != 1 || s->b != 2 || s->c != 3) {
		return 1;
	}

	int one = 1;
	int two = 2;

	s += one;

	if(s->a != 2 || s->b != 3 || s->c != 4) {
		return 1;
	}

	s += two;

	if(s->a != 4 || s->b != 5 || s->c != 6) {
		return 1;
	}

	s -= two;

	if(s->a != 2 || s->b != 3 || s->c != 4) {
		return 1;
	}

	s -= one;

	if(s->a != 1 || s->b != 2 || s->c != 3) {
		return 1;
	}

	return 0;
}

int ints[10];

char chars[10];

struct Struct structs[10];

int test_false_positives(int* i) {
	int a = 0;

	a += 2;
	if(a != 2) {
		return 1;
	}

	a -= 2;
	if(a != 0) {
		return 1;
	}

	a = a + 2;
	if(a != 2) {
		return 1;
	}

	a = a - 2;
	if(a != 0) {
		return 1;
	}

	if(i[0] != 0xFF) {
		return 1;
	}

	i[0] -= 1;
	if(i[0] != 0xFE) {
		return 1;
	}

	i[0] += 1;
	if(i[0] != 0xFF) {
		return 1;
	}

	return 0;
}

int main() {
	int i;

	for(i = 0; i < 10; i += 1) {
		ints[i] = 0xFF - i;
	}
	if(test_ints(ints) != 0) {
		return 1;
	}

	for(i = 0; i < 10; i += 1) {
		chars[i] = i;
	}
	if(test_chars(chars) != 0) {
		return 1;
	}

	for(i = 0; i < 10; i += 1) {
		structs[i].a = i + 1;
		structs[i].b = i + 2;
		structs[i].c = i + 3;
	}
	if(test_struct(structs) != 0) {
		return 1;
	}

	return 0;
}
