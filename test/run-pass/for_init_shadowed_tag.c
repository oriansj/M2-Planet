/* Copyright (C) 2026 Roman Frołow
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

/* A struct tag and an ordinary identifier live in different namespaces, so a
 * variable may have a tag's name; a for initializer that starts with it is an
 * expression unless a declarator follows the type name. */

struct nt
{
	int v;
};

int main()
{
	struct nt s;
	struct nt* p;
	int nt;
	int sum;
	int i;

	s.v = 0;
	sum = 0;
	for (nt = 0; nt < 3; nt = nt + 1)
	{
		s.v = nt;
		sum = sum + nt;
	}
	if(2 != s.v) return 1;
	if(3 != sum) return 2;

	p = &s;
	p->v = 0;
	i = 0;
	for (p->v = 0; p->v < 2; p->v = p->v + 1) i = i + 1;
	if(2 != i) return 3;

	return 0;
}
