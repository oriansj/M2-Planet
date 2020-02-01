/* Copyright (C) 2020 deesix <deesix@tuta.io>
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

// CONSTANT NULL 0

void* malloc(int size)
{
	asm("SET_X0_FROM_BP" "SUB_X0_16" "DEREF_X0"
	    "SET_X1_FROM_X0"
	    "SET_X0_TO_0" "SET_X8_TO_SYS_BRK" "SYSCALL"
	    "PUSH_X0"
	    "ADD_X0_X1_X0"
	    "PUSH_X0"
	    "SYSCALL"
	    "POP_X1"
	    /* TODO: Compare obtained with requested, as error checking. */
	    "POP_X0"
	    /* TODO: Override to return error if detected by the compare. */
	    );
}
