/* Copyright (C) 2020 Jeremiah Orians
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

int access(char* pathname, int mode)
{
	asm("LOAD_EFFECTIVE_ADDRESS_ebx %8"
	"LOAD_INTEGER_ebx"
	"LOAD_EFFECTIVE_ADDRESS_ecx %4"
	"LOAD_INTEGER_ecx"
	"LOAD_IMMEDIATE_eax %33"
	"INT_80");
}
