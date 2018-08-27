## Copyright (C) 2016 Jeremiah Orians
## This file is part of stage0.
##
## stage0 is free software: you can redistribute it and/or modify
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

int getchar()
{
	asm("LOAD_IMMEDIATE_eax %3"
	"LOAD_IMMEDIATE_ebx %0"
	"PUSH_ebx"
	"COPY_esp_to_ecx"
	"LOAD_IMMEDIATE_edx %1"
	"INT_80"
	"TEST"
	"POP_eax"
	"JUMP_NE8 !FUNCTION_getchar_Done"
	"LOAD_IMMEDIATE_eax %-1"
	":FUNCTION_getchar_Done");
}
