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

/* A call's type is the callee's declared return type, not its last
 * argument's: reading the members of the result and loading its value both
 * need it. */

struct inner
{
	int len;
};

struct outer
{
	struct inner kids;
	int tag;
};

static struct outer one;

static struct outer* pick(struct inner* xs, int i)
{
	(void)xs;
	(void)i;
	return &one;
}

int main()
{
	struct inner tmp;
	struct outer* p;
	int v;

	one.kids.len = 7;
	one.tag = 3;
	tmp.len = 0;

	/* A member access through the call. */
	v = pick(&tmp, 0)->kids.len;
	if(7 != v) return 1;

	/* The value the call returns, kept whole. */
	p = pick(&tmp, 0);
	if(p != &one) return 2;
	if(3 != p->tag) return 3;

	return 0;
}
