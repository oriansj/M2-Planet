#! /bin/bash
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


# An example command for compiling a source file
M2-Planet input.c output.M1

# An exmaple command for converting the M1 file into a Hex2 object file ready for linking
M1-Macro -f x86_defs.M1 -f libc.M1 -f output.M1 --LittleEndian --Architecture 1 -o output.hex2

# An example command for converting the Hex2 object file into a runnable x86 binary with an ELF header
hex2-linker -f ELF-i386.hex2 -f output.hex2 --LittleEndian --Architecture 1 --BaseAddress 0x8048000 -o FINAL_BINARY
