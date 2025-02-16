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

struct T {
	int a;
	int b;
};

int main() {
	int a = 0, b = 1, c = 2, d;
	d = 3;

	if (a != 0) return 1;
	if (b != 1) return 2;
	if (c != 2) return 3;
	if (d != 3) return 4;

	/* ensure variables aren't overlapping */
	b = 3;
	if (a != 0) return 5;
	if (b != 3) return 6;
	if (c != 2) return 7;
	if (d != 3) return 8;

	int e = 5, *f;
	f = &e;

	if(e  != 5) return 9;
	if(*f != 5) return 10;

	*f = 10;
	if(e !=  10) return 11;
	if(*f != 10) return 12;

	struct T t, t2, *t3;
	t.a = 1;
	t.b = 2;

	t2.a = 10;
	t2.b = 11;

	t3 = &t2;

	if(t.a != 1 && t.b != 2) return 13;
	if(t2.a != 10 && t2.b != 11) return 14;
	if(t3->a != 10 && t3->b != 11) return 15;

	int* g, h = 1, *i;
	g = &h;
	i = g;

	if(h != 1) return 16;
	if(*g != 1) return 17;
	if(*i != 1) return 18;

	return 0;
}
