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

int global_int;

enum {
	ZERO,
	ONE,
	TWO,
	THREE,
	ALSO_THREE = THREE,
	SIX = THREE + 3,
	SIZEOF_INT = sizeof(int),
	SIZEOF_GLOBAL = sizeof(global_int),
	TEN = SIX + 1 + 1 + 2,
};

struct SizeTwelve {
	int z[THREE];
};

int global[THREE + 2];

int main() {
	int local[THREE];
	static int static_local[THREE];

	if(sizeof(struct SizeTwelve) != 3 * sizeof(int)) return 1;

	if(SIX != 6) return 2;
	if(sizeof(int) != SIZEOF_INT) return 3;

	if(sizeof(THREE) != sizeof(int)) return 4;

	if(SIZEOF_GLOBAL != sizeof(int)) return 5;
	if(SIZEOF_GLOBAL != sizeof(global_int)) return 6;

	static int static_int;
	if(sizeof(static_int) != sizeof(int)) return 7;

	int local_int;
	if(sizeof(local_int) != sizeof(int)) return 8;

	if(TEN != 10) return 9;

	if(sizeof(local) != sizeof(int) * 3) return 10;
	/* Does not work correctly for global arrays.
	if(sizeof(static_local) != sizeof(int) * 3) return 11;
	if(sizeof(global) != sizeof(int) * 5) return 12;*/
}


