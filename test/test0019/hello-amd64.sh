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
bin/M2-Planet --architecture amd64 -f test/common_amd64/functions/file.c \
	-f test/common_amd64/functions/malloc.c \
	-f functions/calloc.c \
	-f test/common_amd64/functions/exit.c \
	-f functions/match.c \
	-f functions/in_set.c \
	-f functions/numerate_number.c \
	-f functions/file_print.c \
	-f test/test0019/getopt.c \
	--bootstrap-mode \
	-o test/test0019/getopt.M1 || exit 1

# Macro assemble with libc written in M1-Macro
M1 -f test/common_amd64/amd64_defs.M1 \
	-f test/common_amd64/libc-core.M1 \
	-f test/test0019/getopt.M1 \
	--LittleEndian \
	--architecture amd64 \
	-o test/test0019/getopt.hex2 || exit 2

# Resolve all linkages
hex2 -f test/common_amd64/ELF-amd64.hex2 -f test/test0019/getopt.hex2 --LittleEndian --architecture amd64 --BaseAddress 0x00600000 -o test/results/test0019-amd64-binary --exec_enable || exit 3

# Ensure binary works if host machine supports test
if [ "$(get_machine ${GET_MACHINE_FLAGS})" = "amd64" ]
then
	. ./sha256.sh
	# Verify that the resulting file works
	./test/results/test0019-amd64-binary -f test/test0019/input -o test/test0019/proof || exit 4
	out=$(sha256_check test/test0019/proof.answer)
	[ "$out" = "test/test0019/proof: OK" ] || exit 5
fi
exit 0
