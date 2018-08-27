/* Copyright (C) 2016 Jeremiah Orians
 * This file is part of stage0.
 *
 * stage0 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * stage0 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with stage0.  If not, see <http://www.gnu.org/licenses/>.
 */

struct utsname
{
	char** sysname[65];    /* Operating system name (e.g., "Linux") */
	char** nodename[65];   /* Name within "some implementation-defined network" */
	char** release[65];    /* Operating system release (e.g., "2.6.28") */
	char** version[65];    /* Operating system version */
	char** machine[65];    /* Hardware identifier */
};

int uname(struct utsname* unameData)
{
	asm("LOAD_EFFECTIVE_ADDRESS_ebx %4"
	    "LOAD_INTEGER_ebx"
	    "LOAD_IMMEDIATE_eax %109"
	    "INT_80");
}
