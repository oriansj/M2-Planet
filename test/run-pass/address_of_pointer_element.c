/* Copyright (C) 2026 Novavero AI Inc.
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

/* &p[i] must be the address of element i of what p points at, for a pointer
 * held globally, locally or as an argument, and for a global array, which is
 * a cell holding the address of its storage. */

int ga[4];
int* gp;

int get(int* p)
{
	return p[0];
}

int* from_argument(int* p, int i)
{
	return &p[i];
}

int main()
{
	int* lp;
	int la[4];
	ga[2] = 42;
	la[1] = 7;
	gp = ga;
	lp = ga;
	if(get(&gp[2]) != 42) return 1;
	if(get(&lp[2]) != 42) return 2;
	if(get(from_argument(ga, 2)) != 42) return 3;
	if(get(&ga[2]) != 42) return 4;
	if(get(&la[1]) != 7) return 5;
	return 0;
}
