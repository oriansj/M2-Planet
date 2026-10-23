/* Copyright (C) 2026 Roman Frołow
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

/* Prefix ++ and -- on a cell reached through a pointer must change that
 * cell and leave its neighbours alone, through a local pointer, a pointer
 * to a pointer and a global pointer. The subscript on the array itself is
 * the control. Every cell starts with a distinct value, so a change to the
 * wrong cell, or to no cell, shows. The cells are read back through a char
 * pointer.
 *
 * The increment itself may still be as wide as a pointer, which is a
 * separate defect. The values are therefore chosen so that adding or
 * subtracting one never carries or borrows out of the target cell, and the
 * arrays are longer than the cells used, so such an access stays inside
 * them. Cells 8 and 15 are far sentinels, read after every operation, so a
 * wide write that changes more than the target still shows. */
char gbuf[16];
char* g;

int main()
{
	char buf[16];
	char* c = buf;
	char* p;
	char** pp;
	char* d = gbuf;

	c[0] = 10;
	c[1] = 20;
	c[2] = 30;
	c[3] = 40;
	c[8] = 80;
	c[15] = 90;

	/* Control: the subscript on the array. */
	++buf[1];
	if(c[0] != 10) return 1;
	if(c[1] != 21) return 2;
	if(c[2] != 30) return 3;
	if(c[8] != 80) return 4;
	if(c[15] != 90) return 5;
	--buf[1];
	if(c[0] != 10) return 6;
	if(c[1] != 20) return 7;
	if(c[2] != 30) return 8;
	if(c[8] != 80) return 9;
	if(c[15] != 90) return 10;

	p = &buf[1];

	/* The subscript through a pointer names buf[2]. */
	++p[1];
	if(c[1] != 20) return 11;
	if(c[2] != 31) return 12;
	if(c[3] != 40) return 13;
	if(c[8] != 80) return 14;
	if(c[15] != 90) return 15;
	--p[1];
	if(c[1] != 20) return 16;
	if(c[2] != 30) return 17;
	if(c[3] != 40) return 18;
	if(c[8] != 80) return 19;
	if(c[15] != 90) return 20;

	/* The dereference names buf[1]. */
	++*p;
	if(c[0] != 10) return 21;
	if(c[1] != 21) return 22;
	if(c[2] != 30) return 23;
	if(c[8] != 80) return 24;
	if(c[15] != 90) return 25;
	--*p;
	if(c[0] != 10) return 26;
	if(c[1] != 20) return 27;
	if(c[2] != 30) return 28;
	if(c[8] != 80) return 29;
	if(c[15] != 90) return 30;

	pp = &p;

	/* Two dereferences name buf[1] through p. */
	++**pp;
	if(c[0] != 10) return 31;
	if(c[1] != 21) return 32;
	if(c[2] != 30) return 33;
	if(c[8] != 80) return 34;
	if(c[15] != 90) return 35;
	--**pp;
	if(c[0] != 10) return 36;
	if(c[1] != 20) return 37;
	if(c[2] != 30) return 38;
	if(c[8] != 80) return 39;
	if(c[15] != 90) return 40;
	if(p != &buf[1]) return 41;

	d[0] = 11;
	d[1] = 22;
	d[2] = 33;
	d[3] = 44;
	d[8] = 88;
	d[15] = 99;

	/* Pointer arithmetic, not &gbuf[1]: the address of a global array's
	 * element is computed wrongly on master too, a separate defect. */
	g = d + 1;

	/* The dereference of a global pointer names gbuf[1]. */
	++*g;
	if(d[0] != 11) return 42;
	if(d[1] != 23) return 43;
	if(d[2] != 33) return 44;
	if(d[8] != 88) return 45;
	if(d[15] != 99) return 46;
	--*g;
	if(d[0] != 11) return 47;
	if(d[1] != 22) return 48;
	if(d[2] != 33) return 49;
	if(d[8] != 88) return 50;
	if(d[15] != 99) return 51;

	/* The subscript through a global pointer names gbuf[2]. */
	++g[1];
	if(d[1] != 22) return 52;
	if(d[2] != 34) return 53;
	if(d[3] != 44) return 54;
	if(d[8] != 88) return 55;
	if(d[15] != 99) return 56;
	--g[1];
	if(d[1] != 22) return 57;
	if(d[2] != 33) return 58;
	if(d[3] != 44) return 59;
	if(d[8] != 88) return 60;
	if(d[15] != 99) return 61;
	if(g != d + 1) return 62;

	return 0;
}
