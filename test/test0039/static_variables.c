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

/* In order to work around https://github.com/oriansj/M2-Planet/issues/63
 * this files uses HIGHLY ILLEGAL undefined behavior by not initializing the statics
 * using the knowledge that M2-Planet automatically zero initializes them.
 * These should be zero initialized explicitly when the issue is fixed.
 */

char* buffer() {
	static char buf[8];

	buf[0] = 0;
	buf[1] = 1;
	buf[2] = 2;
	buf[3] = 3;
	buf[4] = 4;
	buf[5] = 5;
	buf[6] = 6;
	buf[7] = 7;

	return buf;
}

/* Global variable with same name as statics */
int a = 0xFF;

int same_value() {
	static int a /* = 0 */;
	return a;
}

int increment_static() {
	static int a /* = 0 */;

	a = a + 1;

	return a;
}

int main() {
	if(same_value() != 0) return 1;

	int inc = increment_static();
	if(inc != 1) return 2;
	if(increment_static() != 2) return 3;
	if(increment_static() != 3) return 4;

	if(same_value() != 0) return 5;

	char* b = buffer();
	if(b[0] != 0) return 6;
	if(b[1] != 1) return 7;
	if(b[2] != 2) return 8;
	if(b[3] != 3) return 9;
	if(b[4] != 4) return 10;
	if(b[5] != 5) return 11;
	if(b[6] != 6) return 12;
	if(b[7] != 7) return 13;

	b[0] = 0;
	b[1] = 0;
	b[2] = 0;
	b[3] = 0;
	b[4] = 0;
	b[5] = 0;
	b[6] = 0;
	b[7] = 0;

	b = buffer();
	if(b[0] != 0) return 14;
	if(b[1] != 1) return 15;
	if(b[2] != 2) return 16;
	if(b[3] != 3) return 17;
	if(b[4] != 4) return 18;
	if(b[5] != 5) return 19;
	if(b[6] != 6) return 20;
	if(b[7] != 7) return 21;

	if(a != 0xFF) return 22;
	if(same_value() != 0) return 23;
	if(same_value() != 0) return 24;
	if(increment_static() != 4) return 25;
	if(a != 0xFF) return 26;

	return 0;
}

