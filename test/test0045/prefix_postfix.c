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

int for_loop() {
	int a = 0;
	int i;
	for(i = 0; i < 5; ++i) {
		++a;
	}
	if(a != 5) return 1;
	if(i != 5) return 1;

	for(i = 5; i > 0; --i) {
		--a;
	}
	if(a != 0) return 1;
	if(i != 0) return 1;

	for(i = 0; i < 5; i++) {
		a++;
	}
	if(a != 5) return 1;
	if(i != 5) return 1;

	for(i = 5; i > 0; i--) {
		a--;
	}
	if(a != 0) return 1;
	if(i != 0) return 1;

	return 0;
}

int while_loop() {
	int a = 0;
	int i = 0;

	while(++i < 5) {
		++a;
	}

	/* When i == 5 a won't be incremented */
	if(a != 4) return 1;
	if(i != 5) return 1;

	while(--i > 0) {
		--a;
	}

	/* When i == 0 a won't be decremented */
	if(a != 0) return 1;
	if(i != 0) return 1;

	while(i++ < 5) {
		a++;
	}

	if(a != 5) return 1;
	if(i != 6) return 1;

	while(i-- > 0) {
		a--;
	}

	if(a != -1) return 1;
	if(i != -1) return 1;

	return 0;
}

int do_while_loop() {
	int a = 0;
	int i = 0;

	do {
		++a;
	}
	while(++i < 5);
	if(a != 5) return 1;
	if(i != 5) return 1;

	do {
		--a;
	}
	while(--i > 0);
	if(a != 0) return 1;
	if(i != 0) return 1;

	do {
		a++;
	}
	while(i++ < 5);
	if(a != 6) return 1;
	if(i != 6) return 1;

	do {
		a--;
	}
	while(i-- > 0);
	if(a != -1) return 1;
	if(i != -1) return 1;

	return 0;
}

struct T {
	int a;
	int b;
};

int pointers(int* a, char* b, struct T* t) {
	a[0] = 10;
	a[1] = 11;
	a[2] = 12;

	if(*a != 10) return 1;
	++a;
	if(*a != 11) return 1;
	++a;
	if(*a != 12) return 1;
	--a;
	if(*a != 11) return 1;
	--a;
	if(*a != 10) return 1;

	b[0] = 13;
	b[1] = 14;
	b[2] = 15;

	if(*b != 13) return 1;
	++b;
	if(*b != 14) return 1;
	++b;
	if(*b != 15) return 1;
	--b;
	if(*b != 14) return 1;
	--b;
	if(*b != 13) return 1;

	t[0].a = 16;
	t[0].b = 17;
	t[1].a = 18;
	t[1].b = 19;
	t[2].a = 20;
	t[2].b = 21;

	if(t->a != 16) return 1;
	if(t->b != 17) return 1;
	++t;
	if(t->a != 18) return 1;
	if(t->b != 19) return 1;
	++t;
	if(t->a != 20) return 1;
	if(t->b != 21) return 1;
	--t;
	if(t->a != 18) return 1;
	if(t->b != 19) return 1;
	--t;
	if(t->a != 16) return 1;
	if(t->b != 17) return 1;

	return 0;
}

int pointers_postfix(int* a, char* b, struct T* t) {
	a[0] = 10;
	a[1] = 11;
	a[2] = 12;

	if(*a != 10) return 1;
	a++;
	if(*a != 11) return 1;
	a++;
	if(*a != 12) return 1;
	a--;
	if(*a != 11) return 1;
	a--;
	if(*a != 10) return 1;

	b[0] = 13;
	b[1] = 14;
	b[2] = 15;

	if(*b != 13) return 1;
	b++;
	if(*b != 14) return 1;
	b++;
	if(*b != 15) return 1;
	b--;
	if(*b != 14) return 1;
	b--;
	if(*b != 13) return 1;

	t[0].a = 16;
	t[0].b = 17;
	t[1].a = 18;
	t[1].b = 19;
	t[2].a = 20;
	t[2].b = 21;

	if(t->a != 16) return 1;
	if(t->b != 17) return 1;
	t++;
	if(t->a != 18) return 1;
	if(t->b != 19) return 1;
	t++;
	if(t->a != 20) return 1;
	if(t->b != 21) return 1;
	t--;
	if(t->a != 18) return 1;
	if(t->b != 19) return 1;
	t--;
	if(t->a != 16) return 1;
	if(t->b != 17) return 1;

	return 0;
}

int arr[3];
char brr[3];
struct T trr[3];
int main() {
	int a = 0;
	if(++a != 1) return 1;
	if(++a != 2) return 2;

	if(for_loop() != 0) return 3;
	if(while_loop() != 0) return 4;

	int three = ++a;
	if(three != 3) return 5;

	if(do_while_loop() != 0) return 6;

	int i = 0;
	arr[0] = 0;
	arr[++i] = 1;
	arr[++i] = 2;

	if(arr[0] != 0) return 7;
	if(arr[1] != 1) return 8;
	if(arr[2] != 2) return 9;

	if(++arr[0] != 1) return 13;

	if(arr[0] != 1) return 14;

	if(--a != 2) return 15;
	if(--a != 1) return 16;
	if(--a != 0) return 17;

	if(--arr[0] != 0) return 18;

	struct T* trr_ptr = trr;

	if(pointers(arr, brr, trr_ptr) != 0) return 19;
	if(pointers_postfix(arr, brr, trr_ptr) != 0) return 20;

	i = 0;
	arr[i++] = 10;
	arr[i++] = 11;
	arr[i] = 12;

	if(arr[0] != 10) return 21;
	if(arr[1] != 11) return 22;
	if(arr[2] != 12) return 23;

	if(arr[0]++ != 10) return 24;
	if(arr[0] != 11) return 25;

	if(arr[0]-- != 11) return 26;
	if(arr[0] != 10) return 27;

	int b = 0;
	if(b++ != 0) return 28;
	if(b++ != 1) return 29;
	if(b != 2) return 30;
	if(b-- != 2) return 31;
	if(b-- != 1) return 32;
	if(b != 0) return 33;
}

