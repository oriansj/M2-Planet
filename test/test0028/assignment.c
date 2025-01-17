/* Copyright (C) 2021 Andrius Štikonas
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

const int unused;

int main() {
    const int one = 1;
    const int three = 3;
	int a = 44;
	a += one;
	a *= three;
	a /= 4;
	a %= 7;
	a <<= 8;
	a >>= 5;
	a -= 2;
	a &= 11;
	a ^= 9;
	a |= 10;
	return a - 11;
}
