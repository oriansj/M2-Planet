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
bin/M2-Planet --architecture amd64 -f test/test0024/return.c \
	-o test/test0024/return.M1 || exit 1

# Macro assemble with libc written in M1-Macro
M1 -f test/common_amd64/amd64_defs.M1 \
	-f test/common_amd64/libc-core.M1 \
	-f test/test0024/return.M1 \
	--LittleEndian \
	--architecture amd64 \
	-o test/test0024/return.hex2 || exit 2

# Resolve all linkages
hex2 -f test/common_amd64/ELF-amd64.hex2 -f test/test0024/return.hex2 --LittleEndian --architecture amd64 --BaseAddress 0x00600000 -o test/results/test0024-amd64-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "amd64" ]
then
	# Verify that the compiled program returns the correct result
	./test/results/test0024-amd64-binary
	[ 42 = $? ] || exit 3
fi
exit 0
