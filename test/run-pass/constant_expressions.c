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
	TWELVE = THREE * 4,
	STRING_LENGTH = sizeof("1234567"),
	CHARACTER = 'A', /* 0x41 */
	CHARACTER_PLUS = 'A' + 10, /* 0x41 */
};

struct SizeTwelve {
	int z[THREE];
};

struct SizeTwelveBytes {
	char z[TWELVE];
};

int global[THREE + 2];

int main() {
	int local[THREE];
	static int static_local[THREE];

	if(sizeof(struct SizeTwelve) != 3 * sizeof(int)) return 1;
	if(sizeof(struct SizeTwelveBytes) != TWELVE) return 2;

	if(SIX != 6) return 3;
	if(sizeof(int) != SIZEOF_INT) return 4;

	if(sizeof(THREE) != sizeof(int)) return 5;

	if(SIZEOF_GLOBAL != sizeof(int)) return 6;
	if(SIZEOF_GLOBAL != sizeof(global_int)) return 7;

	static int static_int;
	if(sizeof(static_int) != sizeof(int)) return 8;

	int local_int;
	if(sizeof(local_int) != sizeof(int)) return 9;

	if(TEN != 10) return 10;
	if(TWELVE != 12) return 11;

	if(sizeof(local) != sizeof(int) * 3) return 12;
	if(sizeof(static_local) != sizeof(int) * 3) return 13;
	if(sizeof(global) != sizeof(int) * 5) return 14;

	char* local_pointer;

	if(sizeof(local_pointer) != sizeof(void*)) return 15;

	if(sizeof(*local_pointer) != 1) return 16;

	if(STRING_LENGTH != 8) return 17;
	if(sizeof("12345") != 6) return 18;

	if(CHARACTER != 0x41) return 19;
	if(CHARACTER_PLUS != 0x4B) return 20;
}
