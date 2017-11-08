#!/bin/bash
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

# example for compiling example For loop test
M2-Planet for.c for.o
M1-Macro -f x86_defs.M1 -f for.o --LittleEndian --Architecture 1 -o for.hex2
hex2-linker -f cc_lib.hex2 -f for.hex2 --LittleEndian --Architecture 1 --BaseAddress 0x8048000 -o for --exec_enable
./for
