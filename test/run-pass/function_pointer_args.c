/* Copyright (C) 2026 Ben Siraphob
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

typedef int (*FUNCTION)(int);

int table[3];

int indexed_get(int i) {
	return table[i];
}

int identity(int x) {
	return x;
}

int main() {
	FUNCTION fp;

	table[0] = 100;
	table[1] = 200;
	table[2] = 300;

	fp = identity;

	if(fp(table[2]) != 300) return 1;
	if(fp(indexed_get(1)) != 200) return 2;
	if(fp(indexed_get(table[0] - 100)) != 100) return 3;
}
