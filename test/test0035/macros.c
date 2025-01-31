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

#define INCREMENT_NO_ARGS() do { \
		a += 1; \
    } while(0)

#define INCREMENT(variable) do { \
		variable += 1; \
    } while(0)

#define INCREMENT_BY(variable, amount) do { \
		variable += amount; \
    } while(0)

/* Not a function-like macro */
#define NEGATIVE_ONE (-1)

int internal_line1() {
	return __LINE__;
}

int internal_line2() {
	return __LINE__;
}

int compare_string(char* a, char* b) {
	int i = 0;
	while (a[i] != 0) {
		if(a[i] != b[i]) {
			return 1;
		}

		i += 1;
	}

	return 0;
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

	INCREMENT(a);

	if(a != 3) {
		return 1;
	}

	INCREMENT_BY(a, 2);

	if(a != 5) {
		return 1;
	}

	INCREMENT_NO_ARGS();

	if(a != 6) {
		return 1;
	}

	a += NEGATIVE_ONE;

	if(a != 5) {
		return 1;
	}

	if (b != 1)
	{
		return 1;
	}

	if (internal_line1() != 47) {
		return 1;
	}

	if (internal_line2() != 51) {
		return 1;
	}

	if(__LINE__ != 122) {
		return 1;
	}

	char* filename = __FILE__;
	char* expected = "test/test0035/macros.c";

	if(compare_string(filename, expected) != 0) {
		return 2;
	}

	char* date = __DATE__;
	char* expected_date = "Jan  1 1970";
	if(compare_string(date, expected_date) != 0) {
		return 3;
	}

	char* time = __TIME__;
	char* expected_time = "00:00:00";
	if(compare_string(time, expected_time) != 0) {
		return 4;
	}

#ifndef __STDC__
	return 5;
#endif

#ifndef __STDC_HOSTED__
	return 6;
#endif

#ifndef __STDC_VERSION__
	return 7;
#endif

	return 0;
}
