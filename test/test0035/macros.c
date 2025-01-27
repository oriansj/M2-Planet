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

#define A int a = 0;
#define B a += 1;
#define C int b = 0; b += 1;
#define D do { \
        if(b != 1 ) { \
            return 1; \
        } \
    } while(0)

#define INCREMENT_A do { \
		a += 1; \
    } while(0)

int internal_line1() {
	return __LINE__;
}

int internal_line2() {
	return __LINE__;
}

int main()
{
	A
	B
	C
	D;

	if (a != 1)
	{
		return 1;
	}

	INCREMENT_A;

	if(a != 2) {
		return 1;
	}

	if (b != 1)
	{
		return 1;
	}

	if (internal_line1() != 32) {
		return 1;
	}

	if (internal_line2() != 36) {
		return 1;
	}

	if(__LINE__ != 70) {
		return 1;
	}

	return 0;
}
