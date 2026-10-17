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

/* A global string initializer's storage label is numbered, so a string with a
 * space, a quote or a newline in it still produces a label the assembler
 * accepts. Two initializers must get different storage. */

static char* text = "hello world\n";
static char* quoted = "a \"b\" c";
static char* other = "second string";

int main()
{
	if('h' != text[0]) return 1;
	if(' ' != text[5]) return 2;
	if('\n' != text[11]) return 3;
	if('a' != quoted[0]) return 4;
	if('"' != quoted[2]) return 5;
	if('s' != other[0]) return 6;
	if('d' != other[12]) return 7;
	return 0;
}
