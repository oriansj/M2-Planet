#! /usr/bin/env bash
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

set -eux
[ -e ../bin/M2-Planet ] && rm ../bin/M2-Planet

# Make the required bin directry
[ -d ../bin ] || mkdir ../bin

# Macro assemble with libc written in M1-Macro
M1 -f x86/cc_x86.M1 \
	--LittleEndian \
	--architecture x86 \
	-o seed.hex2 || exit 1

# Resolve all linkages
hex2 -f ../test/common_x86/ELF-i386.hex2 \
	-f seed.hex2 \
	--LittleEndian \
	--architecture x86 \
	--BaseAddress 0x8048000 \
	-o cc_x86 \
	--exec_enable || exit 2

# Build M2-Planet from cc_x86
catm hold ../test/common_x86/functions/file.c \
	../test/common_x86/functions/malloc.c \
	../functions/calloc.c \
	../test/common_x86/functions/exit.c \
	../functions/match.c \
	../functions/in_set.c \
	../functions/numerate_number.c \
	../functions/file_print.c \
	../functions/number_pack.c \
	../functions/fixup.c \
	../functions/string.c \
	../cc.h \
	../cc_reader.c \
	../cc_strings.c \
	../cc_types.c \
	../cc_core.c \
	../cc.c
./cc_x86 hold M2.M1
M1 --architecture x86 \
	--LittleEndian \
	-f ../test/common_x86/x86_defs.M1 \
	-f ../test/common_x86/libc-core.M1 \
	-f M2.M1 \
	-o M2.hex2

hex2 -f ../test/common_x86/ELF-i386.hex2 \
	-f M2.hex2 \
	--LittleEndian \
	--architecture x86 \
	--BaseAddress 0x8048000 \
	-o ../bin/M2-Planet-seed \
	--exec_enable
