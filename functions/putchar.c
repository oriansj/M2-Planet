## Copyright (C) 2016 Jeremiah Orians
## This file is part of stage0.
##
## stage0 is free software: you an redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## stage0 is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with stage0.  If not, see <http://www.gnu.org/licenses/>.

void putchar(int c)
{
	asm("LOAD_IMMEDIATE_eax %4"
	"LOAD_IMMEDIATE_ebx %1"
	"LOAD_EFFECTIVE_ADDRESS_ecx %4"
	"COPY_ebx_to_edx"
	"INT_80");
}
