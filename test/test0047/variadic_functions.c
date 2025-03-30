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

typedef __va_list va_list;

/* M2-Planet still has problems with function-like macros. */
#define va_start __va_start
#define va_arg __va_arg
#define va_end __va_end
#define va_copy(ap1, ap2) ap2 = ap1

int add_numbers(int count, ...)
{
	int result = 0;
	va_list args;
	va_start(args, count);

	int i;
	for (i = 0; i < count; ++i) {
		result += va_arg(args, int);
	}

	va_end(args);

	return result;
}

int simple(int first, ...)
{
	va_list ap;
	va_start(ap, first);

	return va_arg(ap, int);
}

int copy(int count, ...)
{
	int result = 0;
	int second_count = count;
	va_list args;
	va_list second_args;
	va_start(args, count);
	va_copy(args, second_args);

	int i;
	for (i = 0; i < count; ++i) {
		result += va_arg(args, int);
	}

	int i;
	for (i = 0; i < count; ++i) {
		result += va_arg(second_args, int);
	}

	va_end(args);
	va_end(second_args);

	return result;
}

int main() {
	if(simple(1, 2) != 2) return 1;
	if(add_numbers(4, 25, 25, 50, 50) != 150) return 2;
	if(add_numbers(3, 25, 25, 50) != 100) return 3;
	if(add_numbers(2, 25, 25) != 50) return 4;
	if(copy(2, 25, 25) != 100) return 5;
}

