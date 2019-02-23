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

set -ex
# Build the test
bin/M2-Planet -f functions/malloc.c \
	-f functions/calloc.c \
	-f functions/putchar.c \
	-f test/test17/memset.c \
	-o test/test17/memset.M1 || exit 1

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f functions/libc-core.M1 \
	-f test/test17/memset.M1 \
	--LittleEndian \
	--architecture x86 \
	-o test/test17/memset.hex2 || exit 2

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 -f test/test17/memset.hex2 --LittleEndian --architecture x86 --BaseAddress 0x8048000 -o test/results/test17-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the resulting file works
	./test/results/test17-binary >| test/test17/proof || exit 4
	out=$(sha256sum -c test/test17/proof.answer)
	[ "$out" = "test/test17/proof: OK" ] || exit 5
fi
exit 0
