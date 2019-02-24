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
bin/M2-Planet --architecture x86 -f test/common_x86/functions/putchar.c \
	-f test/test06/for.c \
	-o test/test06/for.M1 || exit 1

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f test/common_x86/libc-core.M1 \
	-f test/test06/for.M1 \
	--LittleEndian \
	--architecture x86 \
	-o test/test06/for.hex2 || exit 2

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 -f test/test06/for.hex2 --LittleEndian --architecture x86 --BaseAddress 0x8048000 -o test/results/test06-x86-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "x86" ]
then
	# Verify that the resulting file works
	./test/results/test06-x86-binary >| test/test06/proof || exit 4
	out=$(sha256sum -c test/test06/proof.answer)
	[ "$out" = "test/test06/proof: OK" ] || exit 5
fi
exit 0
