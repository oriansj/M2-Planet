/* Copyright (C) 2022 Andrius Štikonas
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

struct s
{
	char x;
	int y;
	int z[3];
};

struct s a;

int main() {
	a.x = 3;
	a.y = 5;
	if(a.x * a.y != 15)
	{
	    return 1;
	}
	if((&a)->y != 5)
	{
	    return 2;
	}
	a.z[0] = 1;
	a.z[1] = 2;
	a.z[2] = 3;
	return a.z[0] + a.z[1] + a.z[2] - 6;
}
