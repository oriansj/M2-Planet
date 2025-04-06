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

typedef void (*FUNCTION)(void);
typedef int (*MyFunction)(int, int);
typedef int (*MyFunctionArgs)(int first_arg, int second_arg);
typedef int (*MyFunc)(int*, int**);
typedef int (*MyFuncArgs)(int* first_arg, int** second_arg);
typedef int (*MyFuncFp)(int* first_arg, int** second_arg, void(*third_arg)(void), void(*)(void));

static int a;

static int add_static(int a) { return a + a; }

inline int add_inline(int c) { return c + c; }

static inline int add_both(int x) { return x + x; }

inline static int add_reversed(int b) { return b + b; }

_Noreturn void do_not_return();

void (*global_function_pointer)(void);
void (*global_int_fp)(int, int);
void (*global_pointer_fp)(int*, int*);
void (*global_int_name_fp)(int first_arg, int second_arg);
void (*global_pointer_name_fp)(int* first_arg, int* second_arg);
void (*global_pointer_fp_fp)(int* first_arg, int* second_arg, void (*third_arg)(void), void (*)(void));

struct {
	void (*f)(void);
	void (*f_int)(int, int);
	void (*f_pointer)(int*, int*);
	void (*f_name_int)(int first_arg, int second_arg);
	void (*f_pointer_name)(int* first_arg, int* second_arg);
	void (*f_pointer_fp)(int* first_arg, int* second_arg, void (*third_arg)(void), void (*)(void));
};

int main() {
	a = 1;

	if(2 != add_static(1)) return 1;
	if(2 != add_both(1)) return 2;
	if(2 != add_reversed(1)) return 3;
	if(2 != add_inline(1)) return 4;

	if(a != 1) return 5;

	MyFunction my_function;
	MyFunc my_func;
	MyFuncArgs my_func_args;
	MyFunctionArgs my_function_args;
	MyFuncFp my_func_fp;

	void (*function_pointer)(void);
	void (*int_fp)(int, int);
	void (*pointer_fp)(int*, int*);
	void (*int_name_fp)(int first_arg, int second_arg);
	void (*pointer_name_fp)(int* first_arg, int* second_arg);
	void (*pointer_fp_fp)(int* first_arg, int* second_arg, void (*third_arg)(void), void (*)(void));

	return 0;
}

