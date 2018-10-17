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
bin/M2-Planet -f functions/putchar.c \
	-f functions/exit.c \
	-f test/test12/break-for.c \
	-o test/test12/break-for.M1 || exit 1

# Macro assemble with libc written in M1-Macro
M1 -f test/common_x86/x86_defs.M1 \
	-f functions/libc-core.M1 \
	-f test/test12/break-for.M1 \
	--LittleEndian \
	--Architecture 1 \
	-o test/test12/break-for.hex2 || exit 2

# Resolve all linkages
hex2 -f test/common_x86/ELF-i386.hex2 -f test/test12/break-for.hex2 --LittleEndian --Architecture 1 --BaseAddress 0x8048000 -o test/results/test12-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine)" = "x86_64" ]
then
	# Verify that the resulting file works
	./test/results/test12-binary >| test/test12/proof || exit 4
	out=$(sha256sum -c test/test12/proof.answer)
	[ "$out" = "test/test12/proof: OK" ] || exit 5
fi
exit 0
