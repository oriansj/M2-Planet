#! /bin/sh
## Copyright (C) 2017 Jeremiah Orians
## This file is part of M2-Planet.
##
## M2-Planet is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## M2-Planet is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with M2-Planet.  If not, see <http://www.gnu.org/licenses/>.

set -x
# Build the test
./bin/M2-Planet -f test/test26/lisp.h \
	-f functions/malloc.c \
	-f functions/calloc.c \
	-f functions/in_set.c \
	-f functions/numerate_number.c \
	-f functions/match.c \
	-f functions/file.c \
	-f functions/file_print.c \
	-f functions/exit.c \
	-f test/test26/lisp.c \
	-f test/test26/lisp_cell.c \
	-f test/test26/lisp_eval.c \
	-f test/test26/lisp_print.c \
	-f test/test26/lisp_read.c \
	--debug \
	-o test/test26/lisp.M1 || exit 1

# Build debug footer
blood-elf -f test/test26/lisp.M1 \
	-o test/test26/lisp-footer.M1 || exit 2

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f functions/libc-core.M1 \
	-f test/test26/lisp.M1 \
	-f test/test26/lisp-footer.M1 \
	--LittleEndian \
	--Architecture 1 \
	-o test/test26/lisp.hex2 || exit 3

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386-debug.hex2 \
	-f test/test26/lisp.hex2 \
	--LittleEndian \
	--Architecture 1 \
	--BaseAddress 0x8048000 \
	-o test/results/test26-binary \
	--exec_enable || exit 4

exit 0
