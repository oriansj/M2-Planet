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

	return 0;
}

int arr[3];
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

}

